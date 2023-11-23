/*********************************************************
 * Module name: buf_api.c
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
 *    Buffer writing sectors in RAM (e.g. FIFO), until
 *    enough sectors to write as an MPP (multiple plane
 *    page), which can program parallelly. Also force to
 *    flush when stop or non-seqential writing happened.
 *
 *********************************************************/


#include <core\inc\cmn.h>
#include <core\inc\buf.h>
#include <core\inc\ftl.h>

#include <sys\sys.h>


#if defined(__ICCARM__)

#include "lpc313x_chip.h"

/* TODO: place pb_pool on both SRAM0 and SRAM1, and so, USB and DMA can
 * access SRAM together!. How to make them access buffers on different SRAM?
 */
#pragma location = (ISRAM_ESRAM1_BASE-BUFFER_COUNT*sizeof(PAGE_BUFFER)/2)
__no_init
#endif
static PAGE_BUFFER      pb_pool[BUFFER_COUNT];

static BOOL             pb_pool_used[BUFFER_COUNT];

static BOOL             sector_written[SECTOR_PER_MPP];

static void*            write_buffer;
static void*            merge_buffer;

static LSADDR           buf_start_addr;
static LSADDR           buf_end_addr;


void BUF_Init()
{
   UINT32   i;

   /* init the rambuffer variables */
   buf_start_addr = INVALID_LSADDR;
   buf_end_addr = INVALID_LSADDR;

   for (i=0; i<BUFFER_COUNT; i++)
   {
      pb_pool_used[i] = FALSE;
   }

   for (i=0; i<SECTOR_PER_MPP; i++)
   {
      sector_written[i] = FALSE;
   }

   write_buffer = BUF_Allocate();
   merge_buffer = NULL;
}


void BUF_PutSector(LSADDR addr, void* sector)
{
   ASSERT(write_buffer != NULL);

   if (buf_end_addr == INVALID_LSADDR)
   {
      buf_start_addr = addr & (~(SECTOR_PER_MPP-1));
      buf_end_addr = addr | (SECTOR_PER_MPP-1);
   }

   if (addr >= buf_start_addr && addr <= buf_end_addr)
   {
      /* can put to ram write_buffer */
      memcpy(&(((UINT8*)write_buffer)[(addr-buf_start_addr)*SECTOR_SIZE]),
             sector,
             SECTOR_SIZE);
      sector_written[addr-buf_start_addr] = TRUE;
   }
   else
   {
      /* ASSERT: must call flush between two transcation */
      ASSERT(FALSE);
   }
}


void BUF_GetPage(PGADDR* addr, void** buffer,Boolean flag,Boolean mft)
{
   UINT32   i;
   BOOL     need_merge = FALSE;
   PGADDR   page_addr = buf_start_addr>>SECTOR_PER_MPP_SHIFT;
   STATUS   ret;

   ASSERT(buf_start_addr != INVALID_LSADDR);

   for (i=0; i<SECTOR_PER_MPP; i++)
   {
      if (sector_written[i] == FALSE)
      {
         need_merge = TRUE;
         break;
      }
   }

   if (need_merge == TRUE)
   {
      ASSERT(merge_buffer == NULL);
      merge_buffer = BUF_Allocate();

      /* read back the page and merge with write sectors */
      ret = FTL_Read(page_addr, merge_buffer,flag,mft);
      if (ret == STATUS_SUCCESS)
      {
         for (i=0; i<SECTOR_PER_MPP; i++)
         {
            if (sector_written[i] == TRUE)
            {
               memcpy(&(((UINT8*)merge_buffer)[i*SECTOR_SIZE]),
                      &(((UINT8*)write_buffer)[i*SECTOR_SIZE]),
                      SECTOR_SIZE);
            }
         }
      }

      BUF_Free(write_buffer);
      write_buffer = NULL;
      *buffer = merge_buffer;
      merge_buffer = NULL;
   }
   else
   {
      *buffer = write_buffer;

      /* this write buffer is passed out, and should allocate
       * another write buffer.
       */
      write_buffer = NULL;
   }

   /* return the page address */
   *addr = page_addr;

   /* prepare buffer for following sector writes */
   buf_start_addr = INVALID_LSADDR;
   buf_end_addr = INVALID_LSADDR;

   for (i=0; i<SECTOR_PER_MPP; i++)
   {
      sector_written[i] = FALSE;
   }

   if (write_buffer == NULL)
   {
      /* allocate another buffer for write_buffer is freed earlier */
      write_buffer = BUF_Allocate();
   }
}


void BUF_Free(void* buffer)
{
   UINT32   i;

   for (i=0; i<BUFFER_COUNT; i++)
   {
      if (&((pb_pool[i])[0]) == buffer)
      {
         ASSERT(pb_pool_used[i] != FALSE);
         pb_pool_used[i] = FALSE;
         break;
      }
   }

   if (i == BUFFER_COUNT)
   {
      ;  /* releasing other memory, no action required */
   }
}


void* BUF_Allocate()
{
   UINT32   i;
   void*    ret = NULL;

   /* allocate buffer between IRAM0 and IRAM1 interleavely 在IRAM0和IRAM1之间交错分配缓冲区*/
   for (i=0; i<BUFFER_COUNT/2; i++)
   {
      if (pb_pool_used[i] == FALSE)
      {
         ret = &((pb_pool[i])[0]);
         pb_pool_used[i] = TRUE;
         break;
      }

      if (pb_pool_used[i+BUFFER_COUNT/2] == FALSE)
      {
         ret = &((pb_pool[i+BUFFER_COUNT/2])[0]);
         pb_pool_used[i+BUFFER_COUNT/2] = TRUE;
         break;
      }
   }

   return ret;
}


