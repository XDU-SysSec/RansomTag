/*mft*************************************************************************
 *
 *   Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2009
 *
 *    File name   : main.c
 *    Description :
 *
 *
 *COMPATIBILITY
 *=============
 *
 *   The USB Mass storage example project is compatible with Embedded Artsists
 *  LPC313x evaluation board. By default the project is configured to use the
 *  J-Link JTAG interface.
 *
 *CONFIGURATION
 *=============
 *
 *  The Project contains the following configurations:
 *
 *  Debug: run in iRAM
 *
 *
 *    History :
 *    1. Date        : 22.8.2009
 *       Author      : Stanimir Bonev
 *       Description : initial revision.
 *
 *    $Revision: 32285 $
 **************************************************************************/

/** include files **/
#include "includes.h"

#include "lpc313x_timer_driver.h"
#include "lpc313x_usbotg.h"
#include "lpc313x_usbd_driver.h"
#include "lpc313x_chip.h"
#include "mscuser.h"
#include "usbcore.h"
#include "usbhw.h"
#include <onfm.h>
#include <core\inc\cmn.h>
#include <core\inc\buf.h>
#include <core\inc\mtd.h>

#include <recovery\recovery.h> //ransomtag

/** external functions **/
extern void InitSDRAM(void);

/** internal functions **/
extern void USB_EndPoint0(UNS_32 event);

#pragma data_alignment = DMA_BURST_BYTES
unsigned char sector_buffer[SECTOR_SIZE];

#pragma data_alignment = DMA_BURST_BYTES
unsigned char read_sector_buffer[SECTOR_SIZE];

#pragma data_alignment = DMA_BURST_BYTES
UINT8 write_page_buffer[MPP_SIZE];
#pragma data_alignment = DMA_BURST_BYTES
UINT8 read_page_buffer[MPP_SIZE];

#define ISROM_MMU_TTBL (0x1201C000)
#define USER_SPACE_SECTOR_COUNT (ONFM_Capacity())

/*-------------- ransomtag -----------Start*/

unsigned long MFT_Reserved_currentsec = MFT_RESERVED_SECTOR_START; //保留扇区当前未使用区域起始扇区号
unsigned long pre_write_sector = INVALID_LSADDR;
unsigned long pre_read_sector = INVALID_LSADDR;
/*-------------- ransomtag -------------End*/

/***********************************************************************
 *
 * Function: USB_Reset_Event
 *
 * Purpose: USB Reset Event Callback
 *
 * Processing:
 *     Called automatically on USB Reset Event.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 ***********************************************************************/
void USB_Reset_Event(void)
{
   USB_ResetCore();
}

#if USB_CONFIGURE_EVENT
/***********************************************************************
 *
 * Function: USB_Configure_Event
 *
 * Purpose: USB Configure Event Callback
 *
 * Processing:
 *     Called automatically on USB configure Event.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 ***********************************************************************/
void USB_Configure_Event(void)
{
}
#endif

/***********************************************************************
 *
 * Function: USB_EndPoint1
 *
 * Purpose: USB Endpoint 1 Event Callback
 *
 * Processing:
 *     Called automatically on USB Endpoint 1 Event
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 ***********************************************************************/
void USB_EndPoint1(UNS_32 event)
{
   switch (event)
   {
   case USB_EVT_OUT_NAK:
      MSC_BulkOutNak();
      break;
   case USB_EVT_OUT:
      MSC_BulkOut();
      break;
   case USB_EVT_IN_NAK:
      MSC_BulkInNak();
      break;
   case USB_EVT_IN:
      MSC_BulkIn();
      break;
   }
}

static void init_usb()
{
   LPC_USBDRV_INIT_T usb_cb;

   // Enable USB interrupts
   // Install Interrupt Service Routine, Priority
   INTC_IRQInstall(USB_ISR, IRQ_USB, USB_INTR_PRIORITY, 0);

   /* initilize call back structures */
   memset((void *)&usb_cb, 0, sizeof(LPC_USBDRV_INIT_T));
   usb_cb.USB_Reset_Event = USB_Reset_Event;
   usb_cb.USB_P_EP[0] = USB_EndPoint0;
   usb_cb.USB_P_EP[1] = USB_EndPoint1;
   usb_cb.ep0_maxp = USB_MAX_PACKET0;
   /* USB Initialization */
   USB_Init(&usb_cb);
}

static void init_uart()
{
   UartLineCoding_t UartLineCoding;

   /* start uart */
   UartInit(4);
   // Update the baud rate
   UartLineCoding.dwDTERate = 115200;
   // Update the stop bits number
   UartLineCoding.bStopBitsFormat = UART_ONE_STOP_BIT;
   // Update the parity type
   UartLineCoding.bParityType = UART_NO_PARITY;
   // Update the word width
   UartLineCoding.bDataBits = UART_WORD_WIDTH_8;
   // Set UART line coding
   UartSetLineCoding(UartLineCoding);

   //PRINTF("UART started...\n");
}

static void test_mtd()
{
   STATUS ret;
   PHY_BLOCK block = 9;
   UINT8 ids[5];
   SPARE write_spare;
   SPARE read_spare;

   MTD_Init();

   ret = MTD_ReadID(0, ids, 5);
   ret = MTD_Erase(block);
   ret = MTD_CheckBlock(block);

   memset(write_page_buffer, 0xa5, MPP_SIZE);
   write_spare[0] = 0xf0;
   write_spare[1] = 0x0f;

   for (int block = 0; block < 4 * 20; block++)
   {
      ret = MTD_Erase(block);
      ASSERT(ret == STATUS_SUCCESS);
      for (int page = 0; page < PAGE_PER_PHY_BLOCK; page++)
      {
         ret = MTD_Program(block, page, write_page_buffer, write_spare);
         ASSERT(ret == STATUS_SUCCESS);
         ret = MTD_WaitReady(block);
         ASSERT(ret == STATUS_SUCCESS);
      }
   }

   ret = MTD_Read(block, 0, read_page_buffer, read_spare);
   if (memcmp(read_page_buffer, write_page_buffer, MPP_SIZE) == 0)
   {
      if (read_spare[0] == write_spare[0] &&
          read_spare[1] == write_spare[1])
      {
         ret = MTD_ReadStatus(block);
      }
   }
}

static void test_onfm()
{
   unsigned long start_sector = 0;
   unsigned long sector_count = 0;
   unsigned long rand_seed = 544;
   UINT8 write_data = 0;
   UINT32 i;
   int ret;

   for (i = 0; i < 10000; i++)
   {
      /* seed the randome: no seed to freeze the test case */
      srand(rand() + i + rand_seed);

      start_sector = (unsigned long)(rand() % USER_SPACE_SECTOR_COUNT) & (~(SECTOR_PER_MPP - 1));
      rand_seed = (unsigned long)(rand() % (USER_SPACE_SECTOR_COUNT - start_sector));
      write_data = (UINT8)(rand() % ((UINT8)-1));
      sector_count = 8;

      /* set data */
      memset(sector_buffer, (unsigned char)start_sector, SECTOR_SIZE);

      /* write */
      ret = ONFM_Write(start_sector, sector_count, sector_buffer, FALSE, FALSE);
      if (ret == 0)
      {
         /* read and check */
         ret = ONFM_Read(start_sector, sector_count, read_sector_buffer, FALSE, FALSE);
         if (ret == 0)
         {
            ret = memcmp(sector_buffer, read_sector_buffer, SECTOR_SIZE);
         }
      }

      /* print */
      if (ret != 0)
      {
         //PRINTF("%d:*FAIL* start address: %d, sector count: %d, data: %d \n\r",·
         //       i, start_sector, sector_count, write_data);
         break;
      }
      else
      {
         //PRINTF("%d-PASS. start address: %d, sector count: %d, data: %d. \n\r",
         //      i, start_sector, sector_count, write_data);
      }
   }
}

static void usb_user_task_loop()
{
   while (1)
   {
      if (ut_pop != ut_push)
      {
         if (ut_list[ut_pop].type == UT_WRITE)
         {
            LED_SET(LED2);

            //PRINTF("CBW cdb[6]:%d, write:%d, length:%d \n", ut_list[ut_pop].tag, ut_list[ut_pop].offset, ut_list[ut_pop].length);

            /*-------------- ransomtag -----------Start*/
            int ransomtag;
            ransomtag = CheckTag(ut_list[ut_pop].tag);

            /* 在额外区域备份上一版本的MFT  */
            // Backup(ransomtag, ut_list[ut_pop].offset, ut_list[ut_pop].length);

            /*-------------- ransomtag -------------End*/

            Boolean flag = FALSE;
            Boolean mft = FALSE;
            if (ransomtag == 1)
            {
               // PRINTF("WRITE: This cdw has ransomtag and need to be backup! flag=true\n");
               flag = TRUE;
            }
            if (ransomtag == 2)
            {
               mft = TRUE;
            }
            ONFM_Write(ut_list[ut_pop].offset,
                       ut_list[ut_pop].length,
                       ut_list[ut_pop].buffer,
                       flag,
                       mft);

            LED_CLR(LED2);
         }
         else if (ut_list[ut_pop].type == UT_READ)
         {
            if (Read_BulkLen == 0)
            {
               // PRINTF("\n\n-------------------------------------------\nREAD:sector %d,length %d \n", ut_list[ut_pop].offset,
               //        ut_list[ut_pop].length);
               LED_SET(LED1);

               int ransomtag = CheckTag(ut_list[ut_pop].tag);
               Boolean flag = FALSE;
               Boolean mft = FALSE;
               /* 加一个tag表示读备份的数据 */
               if (ransomtag == 3)
               {
                  //PRINTF("READ: This cdw is recoery program! flag=true\n");
                  flag = TRUE;
               }
               if (ransomtag == 2)
               {
                  mft = TRUE;
               }

               ONFM_Read(ut_list[ut_pop].offset,
                         ut_list[ut_pop].length,
                         ut_list[ut_pop].buffer,
                         flag,
                         mft);

               LED_CLR(LED1);

               /* tell the IN NAK INT the buffer is ready to prime */
               Read_BulkLen = (ut_list[ut_pop].length) * MSC_BlockSize;
            }
         }
         else
         {
            ASSERT(ut_list[ut_pop].type == UT_MERGE);

            if (merge_stage == MERGE_START)
            {
               ONFM_Read(ut_list[ut_pop].offset,
                         ut_list[ut_pop].length,
                         ut_list[ut_pop].buffer,
                         FALSE,
                         FALSE);

               merge_stage = MERGE_FINISH;
            }
         }

         /* next write operation */
         ut_pop = (ut_pop + 1) % UT_LIST_SIZE;
      }
   }
}

/*************************************************************************
 * Function Name: main
 * Parameters: None
 *
 * Return: None
 *
 * Description: Getting Started main loop
 *
 *************************************************************************/
void main(void)
{
   int onfm_ret = 0;

#if 0
   CP15_Mmu(FALSE);            // Disable MMU
   CP15_ICache(TRUE);          // Enable ICache

   CP15_SysProt(FALSE);
   CP15_RomProt(TRUE);
   CP15_SetTtb((Int32U *)ISROM_MMU_TTBL);  //Set translation table base address
   CP15_SetDomain( (DomainManager << 2*1) | (DomainClient << 0)); // Set domains
   CP15_Mmu(TRUE);             // Enable MMU
   CP15_Cache(TRUE);           // Enable ICache,DCache
#endif

#ifndef BOOT_LEVEL_2
   InitClock();
   InitSDRAM();
   InitSPINOR();
#endif

   /* TODO:
    * - test DMA copy in SDRAM w/ and w/o clock enable.
    * - test USB RAMDisk speed
    * - test mtd speed
    * - test ONFM-USB
    * - debug, use K9HAG.
    */

   /*Init Interrupt Controller.
     Arm Vector Copy to beginnint of the IRAM*/
   INTC_Init((Int32U *)ISRAM_ESRAM0_BASE);
   /*Remap IRAM at address 0*/
   SYSCREG_ARM926_901616_LP_SHADOW_POINT = ISRAM_ESRAM0_BASE;

   init_uart();
   init_usb();

   //test_mtd();

   /* init ONFM */
   //onfm_ret = ONFM_Mount();

   onfm_ret = 1;

   if (onfm_ret != 0)
   {
      /* init failed, try to format */

      //PRINTF("init ONFM failed. formatting...\n");
      onfm_ret = ONFM_Format();
      if (onfm_ret == 0)
      {
         onfm_ret = ONFM_Mount();
      }
   }

   //test_onfm();

   if (onfm_ret == 0)
   {
      MSC_Init();

      // Enable USB interrupt
      INTC_IntEnable(IRQ_USB, 1);
      __enable_irq();

      /* wait */
      timer_wait_ms(NULL, 10);

      /* USB Connect */
      USB_Connect(TRUE);
   }

   /* main loop to handle usb read/write tasks in USER SPACE */
   usb_user_task_loop();

   /* TODO: call unmount to flush and check program status
    * periodly after a long time delay. Avoid PLR or unsafe plug-out
    */
   ONFM_Unmount();

   /* TODO: use watchdog timer, to reset system */
}
