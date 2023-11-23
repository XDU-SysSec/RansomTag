/*--------------------------------------------------------------------------
 * U S B  -  K e r n e l
 *--------------------------------------------------------------------------
 * Name:    mscuser.c
 * Purpose: Mass Storage Class Custom User Module
 * Version: V1.20
 *--------------------------------------------------------------------------
 * This software is supplied "AS IS" without any warranties, express,
 * implied or statutory, including but not limited to the implied
 * warranties of fitness for purpose, satisfactory quality and
 * noninfringement. Keil extends you a royalty-free right to reproduce
 * and distribute executable files created using this software for use
 * on NXP ARM microcontroller devices only. Nothing else gives
 * you the right to use this software.
 *
 * Copyright (c) 2008 Keil - An ARM Crane Chu. All rights reserved.
 * Adaption to LPCxxxx, Copyright (c) 2009 NXP.
 *--------------------------------------------------------------------------
 * History:
 *          V1.20 Added SCSI_READ12, SCSI_WRITE12
 *          V1.00 Initial Version
 *--------------------------------------------------------------------------*/

#include "lpc_usb.h"
#include "msc.h"
#include "usbcfg.h"
#include "usbhw.h"
#include "usbcore.h"
#include "mscuser.h"
//#include<drv_uart.h>
#include "drv_uart.h"

#include <string.h>

#include <onfm.h>

#include <core\inc\buf.h>


extern UNS_32 DevStatusFS2HS;

UNS_32   MemOK;                        /* Memory OK */

UNS_32   Offset;                       /* R/W SECTOR Offset */
UNS_32   Length;                       /* R/W SECTOR Length */

UNS_8*   BulkBuf;                      /* Bulk Out Buffer */

UNS_32   BulkLen;                      /* Bulk In/Out Length */
UNS_32   MSC_BlockCount;               /* block count in the volumn image */

UNS_32   Read_BulkLen;

#pragma data_alignment=DMA_BURST_BYTES
UNS_8    Read_BulkBuf[MPP_SIZE];  /* Bulk In Buffer */
#pragma data_alignment=DMA_BURST_BYTES
UNS_8    CMD_BulkBuf[MSC_BlockSize];   /* Bulk In Buffer for commands */
#pragma data_alignment=DMA_BURST_BYTES
MSC_CBW  CBW;                          /* Command Block Wrapper */
#pragma data_alignment=DMA_BURST_BYTES
MSC_CSW  CSW;                          /* Command Status Wrapper */

UNS_8    BulkStage;                    /* Bulk Stage */

UNS_8 getCBW_Flag(){
   return CBW.bmFlags;
}
UNS_8 getCBW_LUN(){
   return CBW.CB[6];
}
UNS_8 getCBW_LUN2(){
   return CBW.CB[0];
}

/* usb transaction list: move write operations to user space */
USB_TRANSCATION  ut_list[UT_LIST_SIZE];
volatile UNS_32  ut_pop;
volatile UNS_32  ut_push;

MERGE_STAGE    merge_stage;
UNS_32         merge_count;

static UNS_32 DataIn_Format(void);
static void DataIn_Transfer(void);


void MSC_Init()
{
   /* allocate memory before 1st bulk */
   BulkBuf = NULL;
   Read_BulkLen = 0;
   merge_stage = MERGE_NONE;
   merge_count = 0;

   /* init the ut_list */
   ut_pop = 0;
   ut_push = 0;

   MSC_BlockCount = ONFM_Capacity();
}


void MSC_SetStallEP (UNS_32 EPNum)            /* set EP halt status according stall status */
{
   USB_SetStallEP(EPNum);
   USB_EndPointHalt  |=  (EPNum & 0x80) ? ((1 << 16) << (EPNum & 0x0F)) : (1 << EPNum);
}


UNS_32 MSC_Reset(void)
{
   USB_EndPointStall = 0x00000000;          /* EP must stay stalled */
   CSW.dSignature = 0;                      /* invalid signature */

   BulkStage = MSC_BS_CBW;
   return (TRUE);
}

// 支持几个盘符；LUN: logic unit number;0表示支持一个盘符
UNS_32 MSC_GetMaxLUN(void)
{
   EP0Buf[0] = 0;               /* No LUN associated with this device */
   return (TRUE);
}


void MSC_MemoryRead(void)
{
   UNS_32   n;    /* sector count */
   int      onfm_ret = 0;

   if (DevStatusFS2HS)
   {
      /* read sectors aligned to a MPP */
      n = MIN(SECTOR_PER_MPP-(Offset%SECTOR_PER_MPP), Length);

      if ((Offset + n) > MSC_BlockCount)
      {
         n = MSC_BlockCount - Offset;
         BulkStage = MSC_BS_DATA_IN_LAST_STALL;
      }
   }
   else
   {
      onfm_ret = -1;
      n = 0;
   }

   if (onfm_ret == 0)
   {
      /* log the read operation to ut_list */
      ut_list[ut_push].type   = UT_READ;
      ut_list[ut_push].offset = Offset;
      ut_list[ut_push].length = n;
      ut_list[ut_push].buffer = Read_BulkBuf;
      ut_list[ut_push].tag = CBW.CB[6];    

      /* handle ONFM read/write in user tasks */
      ut_push = (ut_push+1)%UT_LIST_SIZE;
      /* the ut_list should not be full */
      ASSERT(ut_push != ut_pop);

      Read_BulkLen = 0;

      Offset += n;
      Length -= n;

      CSW.dDataResidue -= (n*MSC_BlockSize);
   }
}


void MSC_MemoryWrite(void)
{
   UNS_32   n;    /* sector count */

   /* BulkLen should be align to sector size */
   if (BulkLen%MSC_BlockSize == 0)
   {
      n = BulkLen/MSC_BlockSize;

      if ((Offset + n) > MSC_BlockCount)
      {
         BulkLen = (MSC_BlockCount - Offset)*MSC_BlockSize;
         BulkStage = MSC_BS_CSW;
         MSC_SetStallEP(MSC_EP_OUT);
      }

      /* log the write operation to ut_list */
      ut_list[ut_push].type   = UT_WRITE;
      ut_list[ut_push].offset = Offset&(~(SECTOR_PER_MPP-1));
      ut_list[ut_push].length = SECTOR_PER_MPP;
      ut_list[ut_push].buffer = BulkBuf;
      ut_list[ut_push].tag = CBW.CB[6];    

      /* handle ONFM read/write in user tasks */
      ut_push = (ut_push+1)%UT_LIST_SIZE;
      /* the ut_list should not be full */
      ASSERT(ut_push != ut_pop);

      Offset += n;
      Length -= n;
      CSW.dDataResidue -= BulkLen;

      if ((Length == 0) || (BulkStage == MSC_BS_CSW))
      {
         CSW.bStatus = CSW_CMD_PASSED;
         MSC_SetCSW();
      }
   }
}


void MSC_MemoryVerify(void)
{
   Offset += Length;
   Length = 0;

   CSW.dDataResidue = 0;
   CSW.bStatus = CSW_CMD_PASSED;

   MSC_SetCSW();
}


UNS_32 MSC_RWSetup(void)
{
   UNS_32 n;

   /* Logical Block Address of First Block */
   n = (CBW.CB[2] << 24) |
       (CBW.CB[3] << 16) |
       (CBW.CB[4] <<  8) |
       (CBW.CB[5] <<  0);

   Offset = n;

   /* Number of Blocks to transfer */
   switch (CBW.CB[0])
   {
      case SCSI_READ10:
      case SCSI_WRITE10:
      case SCSI_VERIFY10:
         n = (CBW.CB[7] <<  8) |
             (CBW.CB[8] <<  0);
         break;

      case SCSI_READ12:
      case SCSI_WRITE12:
         n = (CBW.CB[6] << 24) |
             (CBW.CB[7] << 16) |
             (CBW.CB[8] <<  8) |
             (CBW.CB[9] <<  0);
         break;
   }

   Length = n;

   if (CBW.dDataLength == 0)                /* host requests no data */
   {
      CSW.bStatus = CSW_CMD_FAILED;
      MSC_SetCSW();
      return (FALSE);
   }

   if (CBW.dDataLength != (n*MSC_BlockSize))
   {
      if ((CBW.bmFlags & 0x80) != 0)         /* stall appropriate EP */
      {
         MSC_SetStallEP(MSC_EP_IN);
      }
      else
      {
         MSC_SetStallEP(MSC_EP_OUT);
      }

      CSW.bStatus = CSW_CMD_FAILED;
      MSC_SetCSW();

      return (FALSE);
   }

   return (TRUE);
}


void MSC_TestUnitReady(void)
{

   if (CBW.dDataLength != 0)
   {
      if ((CBW.bmFlags & 0x80) != 0)
      {
         MSC_SetStallEP(MSC_EP_IN);
      }
      else
      {
         MSC_SetStallEP(MSC_EP_OUT);
      }
   }

   CSW.bStatus = CSW_CMD_PASSED;
   MSC_SetCSW();
}


void MSC_RequestSense (void)
{
   if (!DataIn_Format()) return;

   CMD_BulkBuf[ 0] = 0x70;          /* Response Code */
   CMD_BulkBuf[ 1] = 0x00;
   CMD_BulkBuf[ 2] = 0x02;          /* Sense Key */
   CMD_BulkBuf[ 3] = 0x00;
   CMD_BulkBuf[ 4] = 0x00;
   CMD_BulkBuf[ 5] = 0x00;
   CMD_BulkBuf[ 6] = 0x00;
   CMD_BulkBuf[ 7] = 0x0A;          /* Additional Length */
   CMD_BulkBuf[ 8] = 0x00;
   CMD_BulkBuf[ 9] = 0x00;
   CMD_BulkBuf[10] = 0x00;
   CMD_BulkBuf[11] = 0x00;
   CMD_BulkBuf[12] = 0x30;          /* ASC */
   CMD_BulkBuf[13] = 0x01;          /* ASCQ */
   CMD_BulkBuf[14] = 0x00;
   CMD_BulkBuf[15] = 0x00;
   CMD_BulkBuf[16] = 0x00;
   CMD_BulkBuf[17] = 0x00;

   BulkLen = 18;
   DataIn_Transfer();
}


void MSC_Inquiry(void)
{
   if (!DataIn_Format()) return;

   CMD_BulkBuf[ 0] = 0x00;          /* Direct Access Device */
   CMD_BulkBuf[ 1] = 0x80;          /* RMB = 1: Removable Medium */
   CMD_BulkBuf[ 2] = 0x00;          /* Version: No conformance claim to standard */
   CMD_BulkBuf[ 3] = 0x01;

   CMD_BulkBuf[ 4] = 36 - 4;        /* Additional Length */
   CMD_BulkBuf[ 5] = 0x80;          /* SCCS = 1: Storage Controller Component */
   CMD_BulkBuf[ 6] = 0x00;
   CMD_BulkBuf[ 7] = 0x00;

   CMD_BulkBuf[ 8] = 'C';           /* Vendor Identification */
   CMD_BulkBuf[ 9] = 'r';
   CMD_BulkBuf[10] = 'a';
   CMD_BulkBuf[11] = 'n';
   CMD_BulkBuf[12] = 'e';
   CMD_BulkBuf[13] = '5';
   CMD_BulkBuf[14] = '4';
   CMD_BulkBuf[15] = '4';

   CMD_BulkBuf[16] = 'O';           /* Product Identification */
   CMD_BulkBuf[17] = 'p';
   CMD_BulkBuf[18] = 'e';
   CMD_BulkBuf[19] = 'n';
   CMD_BulkBuf[20] = ' ';
   CMD_BulkBuf[21] = 'N';
   CMD_BulkBuf[22] = 'A';
   CMD_BulkBuf[23] = 'N';
   CMD_BulkBuf[24] = 'D';
   CMD_BulkBuf[25] = 'F';
   CMD_BulkBuf[26] = ' ';
   CMD_BulkBuf[27] = 'M';
   CMD_BulkBuf[28] = 'g';
   CMD_BulkBuf[29] = 'r';
   CMD_BulkBuf[30] = ' ';
   CMD_BulkBuf[31] = ' ';

   CMD_BulkBuf[32] = '0';           /* Product Revision Level */
   CMD_BulkBuf[33] = '.';
   CMD_BulkBuf[34] = '1';
   CMD_BulkBuf[35] = ' ';

   BulkLen = 36;
   DataIn_Transfer();
}


void MSC_ModeSense6(void)
{
   if (!DataIn_Format()) return;

   CMD_BulkBuf[ 0] = 0x03;
   CMD_BulkBuf[ 1] = 0x00;
   CMD_BulkBuf[ 2] = 0x00;
   CMD_BulkBuf[ 3] = 0x00;

   BulkLen = 4;
   DataIn_Transfer();
}


void MSC_ModeSense10(void)
{
   if (!DataIn_Format()) return;

   CMD_BulkBuf[ 0] = 0x00;
   CMD_BulkBuf[ 1] = 0x06;
   CMD_BulkBuf[ 2] = 0x00;
   CMD_BulkBuf[ 3] = 0x00;
   CMD_BulkBuf[ 4] = 0x00;
   CMD_BulkBuf[ 5] = 0x00;
   CMD_BulkBuf[ 6] = 0x00;
   CMD_BulkBuf[ 7] = 0x00;

   BulkLen = 8;
   DataIn_Transfer();
}


void MSC_ReadCapacity(void)
{

   if (!DataIn_Format()) return;

   /* Last Logical Block */
   CMD_BulkBuf[ 0] = ((MSC_BlockCount - 1) >> 24) & 0xFF;
   CMD_BulkBuf[ 1] = ((MSC_BlockCount - 1) >> 16) & 0xFF;
   CMD_BulkBuf[ 2] = ((MSC_BlockCount - 1) >>  8) & 0xFF;
   CMD_BulkBuf[ 3] = ((MSC_BlockCount - 1) >>  0) & 0xFF;

   /* Block Length */
   CMD_BulkBuf[ 4] = (MSC_BlockSize >> 24) & 0xFF;
   CMD_BulkBuf[ 5] = (MSC_BlockSize >> 16) & 0xFF;
   CMD_BulkBuf[ 6] = (MSC_BlockSize >>  8) & 0xFF;
   CMD_BulkBuf[ 7] = (MSC_BlockSize >>  0) & 0xFF;

   BulkLen = 8;
   DataIn_Transfer();
}


void MSC_ReadFormatCapacity(void)
{
   if (!DataIn_Format()) return;

   CMD_BulkBuf[ 0] = 0x00;
   CMD_BulkBuf[ 1] = 0x00;
   CMD_BulkBuf[ 2] = 0x00;
   CMD_BulkBuf[ 3] = 0x08;          /* Capacity List Length */

   /* Block Count */
   CMD_BulkBuf[ 4] = (MSC_BlockCount >> 24) & 0xFF;
   CMD_BulkBuf[ 5] = (MSC_BlockCount >> 16) & 0xFF;
   CMD_BulkBuf[ 6] = (MSC_BlockCount >>  8) & 0xFF;
   CMD_BulkBuf[ 7] = (MSC_BlockCount >>  0) & 0xFF;

   /* Block Length */
   CMD_BulkBuf[ 8] = 0x02;          /* Descriptor Code: Formatted Media */
   CMD_BulkBuf[ 9] = (MSC_BlockSize >> 16) & 0xFF;
   CMD_BulkBuf[10] = (MSC_BlockSize >>  8) & 0xFF;
   CMD_BulkBuf[11] = (MSC_BlockSize >>  0) & 0xFF;

   BulkLen = 12;
   DataIn_Transfer();
}


void MSC_GetCBW(void)
{
   if ((BulkLen == sizeof(CBW)) && (CBW.dSignature == MSC_CBW_Signature))
   {
      /* Valid CBW */
      CSW.dTag = CBW.dTag;
      CSW.dDataResidue = CBW.dDataLength;
      if ((CBW.bLUN != 0)     ||
          (CBW.bCBLength < 1) ||
          (CBW.bCBLength > 16) )
      {
fail:
         CSW.bStatus = CSW_CMD_FAILED;
         MSC_SetCSW();
      }
      else
      {
         switch (CBW.CB[0])
         {
            case SCSI_TEST_UNIT_READY:
               MSC_TestUnitReady();
               break;
            case SCSI_REQUEST_SENSE:
               MSC_RequestSense();
               break;
            case SCSI_FORMAT_UNIT:
               goto fail;
            case SCSI_INQUIRY:
               MSC_Inquiry();
               break;
            case SCSI_START_STOP_UNIT:
               goto fail;
            case SCSI_MEDIA_REMOVAL:
               goto fail;
            case SCSI_MODE_SELECT6:
               goto fail;
            case SCSI_MODE_SENSE6:
               MSC_ModeSense6();
               break;
            case SCSI_MODE_SELECT10:
               goto fail;
            case SCSI_MODE_SENSE10:
               MSC_ModeSense10();
               break;
            case SCSI_READ_FORMAT_CAPACITIES:
               MSC_ReadFormatCapacity();
               break;
            case SCSI_READ_CAPACITY:
               MSC_ReadCapacity();
               break;
            case SCSI_READ10:
            case SCSI_READ12:
               if (MSC_RWSetup())
               {
                  if ((CBW.bmFlags & 0x80) != 0)
                  {
                     BulkStage = MSC_BS_DATA_IN;
                     MSC_MemoryRead();
                  }
                  else
                  {
                     MSC_SetStallEP(MSC_EP_OUT);
                     CSW.bStatus = CSW_PHASE_ERROR;
                     MSC_SetCSW();
                  }
               }
               break;
            case SCSI_WRITE10:
            case SCSI_WRITE12:
               if (MSC_RWSetup())
               {
                  if ((CBW.bmFlags & 0x80) == 0)
                  {
                     BulkStage = MSC_BS_DATA_OUT;
                  }
                  else
                  {
                     MSC_SetStallEP(MSC_EP_IN);
                     CSW.bStatus = CSW_PHASE_ERROR;
                     MSC_SetCSW();
                  }
               }
               break;
            case SCSI_VERIFY10:
               if ((CBW.CB[1] & 0x02) == 0)
               {
                  // BYTCHK = 0 -> CRC Check (not implemented)
                  CSW.bStatus = CSW_CMD_PASSED;
                  MSC_SetCSW();
                  break;
               }

               if (MSC_RWSetup())
               {
                  if ((CBW.bmFlags & 0x80) == 0)
                  {
                     BulkStage = MSC_BS_DATA_OUT;
                     MemOK = TRUE;
                  }
                  else
                  {
                     MSC_SetStallEP(MSC_EP_IN);
                     CSW.bStatus = CSW_PHASE_ERROR;
                     MSC_SetCSW();
                  }
               }
               break;
            default:
               goto fail;
         }
      }
   }
   else
   {
      /* Invalid CBW */
      MSC_SetStallEP(MSC_EP_IN);
      /* set EP to stay stalled */
      USB_EndPointStall |=  (1 << (16 + (MSC_EP_IN  & 0x0F)));
      MSC_SetStallEP(MSC_EP_OUT);
      /* set EP to stay stalled */
      USB_EndPointStall |=  (1 << MSC_EP_OUT);
      BulkStage = MSC_BS_ERROR;
   }
}


void MSC_SetCSW(void)
{
   CSW.dSignature = MSC_CSW_Signature;
   USB_WriteEP(MSC_EP_IN, (UNS_8 *)&CSW, sizeof(CSW));
   BulkStage = MSC_BS_CSW;
}


void MSC_BulkInNak(void)
{
   if (Read_BulkLen != 0)
   {
      /* read buffer is ready to prime */
      USB_WriteEP(MSC_EP_IN, Read_BulkBuf, Read_BulkLen);

      if (Length == 0)
      {
         BulkStage = MSC_BS_DATA_IN_LAST;
      }

      if (BulkStage != MSC_BS_DATA_IN)
      {
         CSW.bStatus = CSW_CMD_PASSED;
      }

      /* TODO: pre-read the next page.
       * - log pre-read to the task list
       * - ONFM read the page out, when USB is sending the current page
       * - ONFM check the pre-read buffer before read it from NAND
       * - ONFM return the address of buffer, avoid another copying. 
       */
   }
}

// 从device到host
void MSC_BulkIn(void)
{
   switch (BulkStage)
   {
      case MSC_BS_DATA_IN:
         switch (CBW.CB[0])
         {
            case SCSI_READ10:
            case SCSI_READ12:
               MSC_MemoryRead();
               break;
         }
         break;
      case MSC_BS_DATA_IN_LAST:
         MSC_SetCSW();
         break;
      case MSC_BS_DATA_IN_LAST_STALL:
         MSC_SetStallEP(MSC_EP_IN);
         MSC_SetCSW();
         break;
      case MSC_BS_CSW:
         BulkStage = MSC_BS_CBW;
         break;
   }
}


void MSC_BulkOutNak(void)
{
   UNS_32   n;
   UNS_32   bulkout_len;
   void*    buffer = NULL;

   if (DevStatusFS2HS)
   {
      if (BulkStage == MSC_BS_DATA_OUT)
      {
         if (merge_stage == MERGE_START)
         {
            /* not prime before get merged data */
            buffer = NULL;
            bulkout_len = 0;
         }
         else if (merge_stage == MERGE_FINISH)
         {
            buffer = BulkBuf+(Offset%SECTOR_PER_MPP)*MSC_BlockSize;
            bulkout_len = merge_count*MSC_BlockSize;

            merge_stage = MERGE_NONE;
            merge_count = 0;
         }
         else
         {
            ASSERT(merge_stage == MERGE_NONE);

            /* try to allocate buffer for next bulk */
            BulkBuf = BUF_Allocate();
            if (BulkBuf != NULL)
            {
               /* sector counts to write in MPP aligned */
               n = MIN(SECTOR_PER_MPP-(Offset%SECTOR_PER_MPP), Length);

               /* merge non-aligned or non-full bulk */
               if (n != SECTOR_PER_MPP)
               {
                  /* log the read-for-merge operation to ut_list */
                  ut_list[ut_push].type   = UT_MERGE;
                  ut_list[ut_push].offset = Offset&(~(SECTOR_PER_MPP-1));
                  ut_list[ut_push].length = SECTOR_PER_MPP;
                  ut_list[ut_push].buffer = BulkBuf;

                  /* handle ONFM read/write in user tasks */
                  ut_push = (ut_push+1)%UT_LIST_SIZE;
                  /* the ut_list should not be full */
                  ASSERT(ut_push != ut_pop);

                  merge_stage = MERGE_START;
                  merge_count = n;

                  /* not prime before get merged data */
                  buffer = NULL;
                  bulkout_len = 0;
               }
               else
               {
                  /* CASE 1: data buffer aligned to MPP */
                  buffer = BulkBuf;
                  bulkout_len = MPP_SIZE;
               }
            }
            else
            {
               /* CASE 2: no buffer avaliable */
               buffer = NULL;
               bulkout_len = 0;
            }
         }
      }
      else if (BulkStage == MSC_BS_CBW)
      {
         /* CASE 3: write data to CBW directly */
         buffer = &CBW;
         bulkout_len = sizeof(CBW);
      }
   }

   if (buffer != NULL)
   {
      USB_ReadReqEP(MSC_EP_OUT, buffer, bulkout_len);
   }
   else
   {
      /* if no buffer avaliable in device, skip priming OUT endpoint, and
       * then, a NAK will be sent to host in next transmittion. At that time,
       * we will try to allocate buffer again here. The buffer may be released
       * due to program completed.
       */
      ;
   }
}

//   
void MSC_BulkOut(void)
{
   BulkLen = USB_ReadEP(MSC_EP_OUT, BulkBuf);
   switch (BulkStage)
   {
      case MSC_BS_CBW:
         MSC_GetCBW();
         break;
      case MSC_BS_DATA_OUT:
         switch (CBW.CB[0])
         {
            case SCSI_WRITE10:
            case SCSI_WRITE12:
               MSC_MemoryWrite();
               break;
            case SCSI_VERIFY10:
               MSC_MemoryVerify();
               break;
         }
         break;
      case MSC_BS_CSW:
         break;
      default:
         MSC_SetStallEP(MSC_EP_OUT);
         CSW.bStatus = CSW_PHASE_ERROR;
         MSC_SetCSW();
         break;
   }
}


static
UNS_32 DataIn_Format(void)
{
   if (CBW.dDataLength == 0)
   {
      CSW.bStatus = CSW_PHASE_ERROR;
      MSC_SetCSW();
      return (FALSE);
   }

   if ((CBW.bmFlags & 0x80) == 0)
   {
      MSC_SetStallEP(MSC_EP_OUT);
      CSW.bStatus = CSW_PHASE_ERROR;
      MSC_SetCSW();
      return (FALSE);
   }

   return (TRUE);
}


static
void DataIn_Transfer(void)
{
   BulkLen = MIN(BulkLen, CBW.dDataLength);
   BulkStage = MSC_BS_DATA_IN_LAST;

   USB_WriteEP(MSC_EP_IN, CMD_BulkBuf, BulkLen);

   CSW.dDataResidue = 0;
   CSW.bStatus = CSW_CMD_PASSED;
}


