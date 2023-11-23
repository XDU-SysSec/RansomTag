/*********************************************************
 * Module name: mtd_nand_sim.c
 *
 * Copyright 2010, 2011. All Rights Reserved, Crane Chu.
 *
 * This file is part of OpenNFM.
 *
 * OpenNFM is free software: you can redistribute it and/or 
 * modify it under the terms of the GNU General Public 
 * License as published by the Free Software Foundation, 
 * either version 3 of the License, or (at your option) any 
 * later version.
 * 
 * OpenNFM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied 
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
 * PURPOSE. See the GNU General Public License for more 
 * details.
 *
 * You should have received a copy of the GNU General Public 
 * License along with OpenNFM. If not, see 
 * <http://www.gnu.org/licenses/>.
 *
 * First written on 2010-01-01 by cranechu@gmail.com
 *
 * Module Description:
 *    simulated nand devices.
 *
 *********************************************************/


#include <core\inc\cmn.h>

#include <sys\sys.h>

#include "mtd_nand.h"


typedef enum {
   SIM_NAND_PROGRAMMED = 0,
   SIM_NAND_ERASED = -1,
} SIM_NAND_STATE;

typedef struct {
   UINT8 main_data[PAGE_SIZE];
   UINT8 spare_data[SPARE_BYTES_IN_PAGE];
   SIM_NAND_STATE    state;
} SIM_COLUMN;

/* 4 planes * 8MB/plane = 32Mbyte per interchip, = 16K page/colume */
typedef SIM_COLUMN      SIM_CHIP[CFG_NAND_ROW_COUNT];

typedef enum {
   STATE_READ,
   STATE_PROGRAM,
   STATE_ERASE,
   STATE_READID,
   STATE_READSTATUS,
   STATE_RESET
} NAND_STATE;


/* 2 interchips * 2 chips */
static SIM_CHIP        sim_nand[CFG_NAND_CHIP_COUNT];

/* a simulated NAND:
 *    512byte sector,
 *    4 sector in page,
 *    2 plane in die,
 *    2 die in interchip
 */
static UINT8 sim_nand_id[] =  {0xff,   /* maker code: sim test */
                               0xff,   /* device code: sim device */
                               0x55,   /* interleave, multiplane, interchip */
                               0x05,   /* 2Kpage, 64Kblock, 16byteOOB */
                               0x08    /* 4 planes, 64Mbit plane */
                              };


/* status byte */
static UINT8         sim_nand_status;
/* log state/addr for cmd execute */
static NAND_STATE    sim_nand_state;
static UINT8         sim_nand_chip;
static UINT16        sim_nand_col_addr;
static UINT32        sim_nand_row_addr;


void NAND_Init()
{
   sim_nand_state = STATE_READ;
   sim_nand_chip = 0;
   sim_nand_col_addr = 0;
   sim_nand_row_addr = 0;

   /* clear all data */
   memset(sim_nand, 0xff, sizeof(sim_nand));
}


void NAND_SelectChip(NAND_CHIP chip)
{
   sim_nand_chip = chip;
}


void NAND_DeSelectChip(NAND_CHIP chip)
{

}


void NAND_SendCMD(NAND_CMD cmd)
{
   switch (cmd)
   {
      case CMD_READ:
               sim_nand_state = STATE_READ;
               break;

      case CMD_READ_COMMIT:
               sim_nand_state = STATE_READ;
               break;

      case CMD_READ_ID:
               sim_nand_state = STATE_READID;
               break;

      case CMD_RESET:
               sim_nand_state = STATE_RESET;
               sim_nand_col_addr = 0;
               sim_nand_row_addr = 0;
               break;

      case CMD_PAGE_PROGRAM:
               sim_nand_state = STATE_PROGRAM;
               break;

      case CMD_PAGE_PROGRAM_COMMIT:
               sim_nand_state = STATE_PROGRAM;
               break;

      case CMD_PROGRAM_FAKE_COMMIT:
               sim_nand_state = STATE_PROGRAM;
               break;

      case CMD_PAGE_FAKE_PROGRAM:
               sim_nand_state = STATE_PROGRAM;
               break;

      case CMD_BLOCK_ERASE:
               sim_nand_state = STATE_ERASE;
               break;

      case CMD_BLOCK_ERASE_COMMIT:
               sim_nand_state = STATE_ERASE;
               break;

      case CMD_RANDOM_DATA_IN:
               /* nothing to do */
               break;

      case CMD_RANDOM_DATA_OUT:
               sim_nand_state = STATE_READ;
               break;

      case CMD_RANDOM_DATA_OUT_COMMIT:
               sim_nand_state = STATE_READ;
               break;

      case CMD_READ_STATUS:
               sim_nand_state = STATE_READSTATUS;
               break;

      case CMD_READ_STATUS_DIE1:
               sim_nand_state = STATE_READSTATUS;
               break;

      case CMD_READ_STATUS_DIE2:
               sim_nand_state = STATE_READSTATUS;
               break;
      default:
               ASSERT(FALSE);
   }
}


void NAND_SendAddr(NAND_COL col, NAND_ROW row, UINT8 col_cycle, UINT8 row_cycle)
{
   SIM_COLUMN*    col_ptr;
   UINT8          i;

   sim_nand_col_addr = col;
   sim_nand_row_addr = row;

   if (sim_nand_state == STATE_ERASE)
   {
      /* erase a block start from the row address */
      ASSERT(sim_nand_state == STATE_ERASE);

      for (i=0; i<PAGE_PER_PHY_BLOCK; i++)
      {
         /* clear a page/column */
         col_ptr = &(sim_nand[sim_nand_chip][sim_nand_row_addr+i]);
         memset(col_ptr, 0xff, sizeof(SIM_COLUMN));
      }
   }
}


void NAND_SendData(unsigned char* buffer, SPARE spare_data)
{
   SIM_COLUMN*    col;

   ASSERT(sim_nand_state == STATE_PROGRAM);
   ASSERT(sim_nand_row_addr < CFG_NAND_ROW_COUNT*CFG_NAND_CHIP_COUNT);

   col = &(sim_nand[sim_nand_chip][sim_nand_row_addr]);
   if (col->state != SIM_NAND_ERASED)
   {
      /* has programmed, fatal error */
      ASSERT(FALSE);
   }
   else
   {
      /* to be programmed */
      col->state = SIM_NAND_PROGRAMMED;
   }

   if (spare_data != NULL)
   {
      memcpy(col->spare_data, spare_data, SPARE_BYTES_IN_PAGE);
   }

   if (buffer != NULL)
   {
      memcpy(col->main_data, buffer, PAGE_SIZE);
   }
}


STATUS NAND_ReceiveData(unsigned char* buffer, SPARE spare_data)
{
   SIM_COLUMN*    col;
   /* return fail when ECC check error */
   STATUS         ret = STATUS_SUCCESS;

   ASSERT(sim_nand_state == STATE_READ || sim_nand_state == STATE_READSTATUS);
   sim_nand_state = STATE_READ;

   col = &(sim_nand[sim_nand_chip][sim_nand_row_addr]);
   
   /* get main data */
   if (buffer != NULL)
   {
      memcpy(buffer, col->main_data, PAGE_SIZE);
   }

   /* get spare data */
   if (spare_data != NULL)
   {
      memcpy(spare_data, col->spare_data, SPARE_BYTES_IN_PAGE);
   }

   if (col->state == SIM_NAND_ERASED)
   {
      /* fail to read un-programmed page, ecc error */
      ret = STATUS_FAILURE;
   }

   return ret;
}


void NAND_ReceiveBytes(UINT8* data_buffer, UINT8 len)
{
   if (sim_nand_state == STATE_READID)
   {
      ASSERT(len == sizeof(sim_nand_id));
      memcpy(data_buffer, sim_nand_id, sizeof(sim_nand_id));
   }
   else if (sim_nand_state == STATE_READSTATUS)
   {
      ASSERT(len == sizeof(sim_nand_status));
      data_buffer[0] = sim_nand_status | NAND_STATUS_READY_BIT;
   }
   else if (sim_nand_state == STATE_READ)
   {
      /* random read, to check the bad block flag */
      ASSERT(sim_nand_col_addr == PAGE_SIZE);
      data_buffer[0] = 0xff;
   }
   else
   {
      ASSERT(FALSE);
   }
}


BOOL NAND_ECCStatus(UINT8* ecc_error_count)
{
   if (ecc_error_count != NULL)
   {
      *ecc_error_count = 1;
   }

   /* we only need to sim and test correctable ecc error cases.
    * If the ECC is uncorrectable, SW can do nothing unless reporting
    * error to host. If the ECC is few, SW also need to do nothing.
    */
   return FALSE;
}


void NAND_WaitRB(NAND_CHIP chip)
{
   return;
}


