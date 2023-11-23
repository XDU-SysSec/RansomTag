/*********************************************************
 * Module name: ubi_api.c
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
 *    Unsorted Block Image.
 *
 *********************************************************/


#include <core\inc\cmn.h>
#include <core\inc\mtd.h>
#include <core\inc\ubi.h>
#include <core\inc\buf.h>

#include <sys\sys.h>

#include "ubi_inc.h"


/* NOTICE:
 * PHY_BLOCK place die (channel/chip/die) index in low bits,
 * one PHY_BLOCK combines blocks in multiple planes.
 */


typedef struct {
   LOG_BLOCK      log_block;
   PHY_BLOCK      phy_block;  /* INVALID_BLOCK for empty slot */
   ERASE_COUNT    ec;
   PAGE_OFF       page;
   void*          buffer;
   SPARE          spare;
} DIE_HOLD_PAGE;

static DIE_HOLD_PAGE dice_hold[TOTAL_DIE_COUNT];
static UINT8         tmp_data_buffer[MPP_SIZE];


static
STATUS ubi_reclaim_badblock(LOG_BLOCK     log_block,
                            PHY_BLOCK     phy_block,
                            ERASE_COUNT   phy_block_ec,
                            PAGE_OFF      bad_page,
                            PHY_BLOCK*    new_phy_block,
                            ERASE_COUNT*  new_ec);

static
UINT32 ubi_find_die_buffer(PHY_BLOCK block);


STATUS UBI_Format()
{
   AREA        area = 0;
   PHY_BLOCK   block_count;
   PHY_BLOCK   remaining_count;
   PHY_BLOCK   total_data_block = 0;
   PHY_BLOCK   fmt_current_block = DATA_FIRST_BLOCK;
   STATUS      ret = STATUS_SUCCESS;

   /* try to read out only the anchor table first for bad block table */
   ret = ANCHOR_Init();
 //  ret = STATUS_FAILURE;
   if (ret != STATUS_SUCCESS)
   {
      PHY_BLOCK   block;

      /* detect bad block and build up the bad block table */
      for (block=0; block<CFG_LOG_BLOCK_COUNT; block++)
      {
         ret = MTD_CheckBlock(block);
         if (ret == STATUS_SUCCESS)
         {
            ret = MTD_Erase(block);
         }

         if (ret != STATUS_SUCCESS)
         {
            ANCHOR_LogBadBlock(block);
         }
      }
   }

   do
   {
      ASSERT(fmt_current_block < DATA_BLOCK_COUNT);
      remaining_count = DATA_BLOCK_COUNT - fmt_current_block;
      block_count = MIN(remaining_count, CFG_PHY_BLOCK_PER_AREA);

      /* formatting area */
      fmt_current_block = AREA_Format(block_count, fmt_current_block, area);
      if (fmt_current_block != INVALID_BLOCK)
      {
         area ++;
         /* TODO: consider bad block. Count read total_data_block in AREA_Format. */
         total_data_block += block_count;
      }
      else
      {
         break;
      }
   } while (fmt_current_block < DATA_BLOCK_COUNT);

   if (fmt_current_block != INVALID_BLOCK)
   {
      /* formatting index */
      fmt_current_block = INDEX_Format(CFG_LOG_BLOCK_COUNT, fmt_current_block);
   }

   if (fmt_current_block != INVALID_BLOCK)
   {
      ASSERT(fmt_current_block == CFG_LOG_BLOCK_COUNT);

      /* formatting anchor */
      ret = ANCHOR_Format(total_data_block);
   }
   else
   {
      /* write anchor table to log bad block table */
      (void)ANCHOR_Format(INVALID_BLOCK);
      ret = STATUS_UBI_FORMAT_ERROR;
   }

   return ret;
}


STATUS UBI_Init()
{
   AREA        area;
   AREA        updating_area;
   UINT32      die;
   PHY_BLOCK   updating_logical_block = INVALID_BLOCK;
   PHY_BLOCK   updating_origin_block = INVALID_BLOCK;
   ERASE_COUNT updating_block_ec = INVALID_EC;
   STATUS      ret;

   // anchor��ʼ��ʧ��
   ret = ANCHOR_Init();
   if (ret == STATUS_SUCCESS)
   {
      /* init/plr index table, and get the plr info of area update */
      ret = INDEX_Init(&updating_logical_block,
                       &updating_origin_block,
                       &updating_block_ec);
   }

   if (ret == STATUS_SUCCESS)
   {
      /* find the updating area */
      if (updating_logical_block != INVALID_BLOCK)
      {
         updating_area = AREA_INDEX(updating_logical_block);
      }
      else
      {
         updating_area = INVALID_AREA;
      }

      for (area=0; area<AREA_COUNT; area++)
      {
         /* read out the area table, and setup the area offset table */
         ret = AREA_Init(area);
         if (ret == STATUS_SUCCESS)
         {
            if (area == updating_area)
            {
               /* check the data of the area for PLR */
               if (AREA_CheckUpdatePLR(updating_logical_block,
                                       updating_origin_block,
                                       updating_block_ec)
                   == TRUE)
               {
                  /* continue to update the area table */
                  INDEX_Update_AreaUpdate(updating_logical_block,
                                          updating_origin_block,
                                          updating_block_ec);
                  ret = INDEX_Update_Commit();
               }
            }
         }
         else
         {
            break;
         }
      }
   }

   if (ret == STATUS_SUCCESS)
   {
      for (die=0; die<TOTAL_DIE_COUNT; die++)
      {
         dice_hold[die].phy_block = INVALID_BLOCK;
      }
   }

   return ret;
}


STATUS UBI_Read(LOG_BLOCK block, PAGE_OFF page, void* buffer, SPARE spare)
{
   PHY_BLOCK   phy_block;
   STATUS      ret = STATUS_SUCCESS;

   if (block != INVALID_BLOCK && page != INVALID_PAGE)
   {
      phy_block = AREA_GetBlock(block);
      ASSERT(phy_block != INVALID_BLOCK);

      /* TODO: handle the read fail issue, or ECC danger issue */
      ret = MTD_Read(phy_block, page, buffer, spare);
   }
   else
   {
      ASSERT(block == INVALID_BLOCK && page == INVALID_PAGE);

      /* read from invalid page, fill the empty buffer all ZERO */
      memset(buffer, 0, MPP_SIZE);
   }

   return ret;
}


STATUS UBI_Write(LOG_BLOCK block, PAGE_OFF page, void* buffer, SPARE spare, BOOL async)
{
   ERASE_COUNT phy_block_ec;
   ERASE_COUNT new_ec;
   PHY_BLOCK   phy_block;
   PHY_BLOCK   new_phy_block = INVALID_BLOCK;
   STATUS      ret;
   UINT32      die_index = 0;

   phy_block = AREA_GetBlock(block);
   phy_block_ec = AREA_GetEC(block);
   ASSERT(phy_block != INVALID_BLOCK);

   /* check the last page status on the same die */
   ret = MTD_WaitReady(phy_block);
   while (ret == STATUS_BADBLOCK)
   {
      /* find the buffer in the same die */
      die_index = ubi_find_die_buffer(phy_block);

      ASSERT(dice_hold[die_index].phy_block != INVALID_BLOCK);

      /* reclaim earlier pages */
      ret = ubi_reclaim_badblock(dice_hold[die_index].log_block,
                                 dice_hold[die_index].phy_block,
                                 dice_hold[die_index].ec,
                                 dice_hold[die_index].page,
                                 &new_phy_block,
                                 &new_ec);
      if (ret == STATUS_SUCCESS &&
          dice_hold[die_index].phy_block != INVALID_BLOCK)
      {
         dice_hold[die_index].phy_block = new_phy_block;
         dice_hold[die_index].ec = new_ec;

         /* write last page in die buffer */
         ret = MTD_Program(new_phy_block,
                           dice_hold[die_index].page,
                           dice_hold[die_index].buffer,
                           dice_hold[die_index].spare);
      }

      if (ret == STATUS_SUCCESS)
      {
         ret = MTD_WaitReady(new_phy_block);
      }
   }

   if (ret == STATUS_SUCCESS &&
       dice_hold[die_index].phy_block != INVALID_BLOCK)
   {
      ASSERT(dice_hold[die_index].buffer != NULL);

      /* release the die buffer */
      BUF_Free(dice_hold[die_index].buffer);
      dice_hold[die_index].buffer = NULL;
      dice_hold[die_index].phy_block = INVALID_BLOCK;
   }

   if (ret == STATUS_SUCCESS && page != INVALID_PAGE)
   {
      ASSERT(buffer != NULL);

      /* write current page */
      ret = MTD_Program(phy_block, page, buffer, spare);
   }

   if (ret == STATUS_SUCCESS && page != INVALID_PAGE)
   {
      ASSERT(buffer != NULL);

      if (async == FALSE)
      {
         ret = MTD_WaitReady(phy_block);
         while (ret == STATUS_BADBLOCK)
         {
            ret = ubi_reclaim_badblock(block,
                                       phy_block,
                                       phy_block_ec,
                                       page,
                                       &new_phy_block,
                                       &new_ec);
            if (ret == STATUS_SUCCESS)
            {
               /* write last page in die buffer */
               ret = MTD_Program(new_phy_block, page, buffer, spare);
            }

            if (ret == STATUS_SUCCESS)
            {
               ret = MTD_WaitReady(new_phy_block);
            }
         }

         ASSERT(ret == STATUS_SUCCESS);
         BUF_Free(buffer);
      }
      else
      {
         /* save in dice_hold */
         dice_hold[die_index].log_block = block;
         dice_hold[die_index].phy_block = phy_block;
         dice_hold[die_index].ec = phy_block_ec;
         dice_hold[die_index].page = page;
         dice_hold[die_index].buffer = buffer;
         dice_hold[die_index].spare[0] = spare[0];
         dice_hold[die_index].spare[1] = spare[1];
      }
   }

   return ret;
}


STATUS UBI_Flush()
{
   UINT32      i;
   STATUS      ret = STATUS_SUCCESS;

   for (i=0; i<TOTAL_DIE_COUNT; i++)
   {
      if (ret == STATUS_SUCCESS)
      {
         /* check the status of a write buffer in one die */
         ret = UBI_Write(dice_hold[i].log_block,
                         INVALID_PAGE,
                         NULL,
                         NULL,
                         TRUE);
      }
   }

   return ret;
}


STATUS UBI_Erase(LOG_BLOCK block, LOG_BLOCK die_index)
{
   STATUS         ret = STATUS_SUCCESS;
   UINT32         die = die_index%TOTAL_DIE_COUNT;
   PHY_BLOCK      phy_block = INVALID_BLOCK;
   ERASE_COUNT    ec = INVALID_EC;

   /* flush the program on the same die with the block to erase */
   ret = UBI_Write(dice_hold[die].log_block, INVALID_PAGE, NULL, NULL, TRUE);
   if (ret == STATUS_SUCCESS)
   {
      ret = INDEX_FreeBlock_Get(die_index%TOTAL_DIE_COUNT, &phy_block, &ec);
   }

   if (ret == STATUS_SUCCESS)
   {
      ASSERT(block != INVALID_BLOCK && ec != INVALID_EC);

      INDEX_FreeBlock_Put(AREA_GetBlock(block), AREA_GetEC(block));
      INDEX_Update_AreaUpdate(block, phy_block, ec);
      ret = INDEX_Update_Commit();
   }

   return ret;
}


STATUS UBI_SWL()
{
   BLOCK_OFF      min_block_offset;
   PHY_BLOCK      min_physical_block;
   ERASE_COUNT    min_block_ec;
   PHY_BLOCK      max_physical_block;
   ERASE_COUNT    max_block_ec;
   PHY_BLOCK      logical_block;
   PAGE_OFF       i;
   STATUS         ret = STATUS_SUCCESS;
   SPARE          spare;

   /* static wear leveling (SWL):
    * pooling one area, and get the block with min EC in the area,
    * exchange it with the max EC block in FBT, if their EC
    * difference is larger than a threshold.
    */
   min_block_offset = AREA_FindMinECBlock(anchor_table.swl_current_area,
                                          &min_physical_block,
                                          &min_block_ec);
   INDEX_FreeBlock_GetMaxECBlock(&max_physical_block, &max_block_ec);

   /* check if SWL is required */
   if (max_physical_block != min_physical_block &&
       max_physical_block != INVALID_BLOCK &&
       min_physical_block != INVALID_BLOCK &&
       max_block_ec != INVALID_EC &&
       min_block_ec != INVALID_EC &&
       max_block_ec > min_block_ec &&
       max_block_ec-min_block_ec > STATIC_WL_THRESHOLD)
   {
      /* erase the new max-ec-block first */
      ret = MTD_Erase(max_physical_block);

      /* copy data from min ec block to max ec block. */
      for (i=0; i<PAGE_PER_PHY_BLOCK; i++)
      {
         if (ret == STATUS_SUCCESS)
         {
            /* may read erased page, so acceptable error happen */
            (void)MTD_Read(min_physical_block, i, tmp_data_buffer, spare);
            ret = MTD_Program(max_physical_block, i, tmp_data_buffer, spare);
         }

         if (ret == STATUS_SUCCESS)
         {
            ret = MTD_WaitReady(max_physical_block);
         }

         if (ret != STATUS_SUCCESS)
         {
            /* SWL fail, area table and free table are no changed,
             * so, nothing to do to recover.
             */
            break;
         }
      }

      if (ret == STATUS_SUCCESS)
      {
         ASSERT(i == PAGE_PER_PHY_BLOCK);
         /* UPDATE area table. no need to touch cache block even exists */
         logical_block = ((anchor_table.swl_current_area) *
                          CFG_PHY_BLOCK_PER_AREA) +
                         min_block_offset;
         INDEX_Update_AreaUpdate(logical_block,
                                 max_physical_block,
                                 max_block_ec);

         /* update the free block table */
         INDEX_FreeBlock_SwapMaxECBlock(min_physical_block, min_block_ec);

         /* update index table (free block, and area index table if need) and
          * area table at once.
          */
         ret = INDEX_Update_Commit();
      }
   }
   else
   {
      /* do SWL in next area in the next cycle */
      anchor_table.swl_current_area = (anchor_table.swl_current_area + 1) %
                                       AREA_COUNT;
   }

   return ret;
}


STATUS UBI_ReadStatus(LOG_BLOCK block)
{
   /* when choose journal blocks, erase free blocks, and choose other
    * blocks in different dice through calling this function to get
    * the status of nand dice.
    *
    * when writing journal blocks, write data to on block, and write other
    * data to different blocks in different dice through calling this function
    * to get the status of block programming, then achieve intereleave
    * programming across all dice.
    */
   PHY_BLOCK   phy_block;
   STATUS      ret = STATUS_SUCCESS;

   phy_block = AREA_GetBlock(block);
   ASSERT(phy_block != INVALID_BLOCK);

   ret = MTD_ReadStatus(phy_block);

   return ret;
}


static
STATUS ubi_reclaim_badblock(LOG_BLOCK     log_block,
                            PHY_BLOCK     phy_block,
                            ERASE_COUNT   phy_block_ec,
                            PAGE_OFF      bad_page,
                            PHY_BLOCK*    new_phy_block,
                            ERASE_COUNT*  new_phy_ec)
{
   PHY_BLOCK   new_block;
   ERASE_COUNT new_ec;
   PAGE_OFF    i;
   STATUS      ret = STATUS_SUCCESS;
   SPARE       spare;

   /* Reclaim Bad Block:
    * - get another free block, if none, return fail
    * - reclaim bad block, copying 0~page-1
    * - add bad block to free block table, with ec+100.
    */

   if (ret == STATUS_SUCCESS)
   {
      ret = INDEX_FreeBlock_Get(phy_block, &new_block, &new_ec);
   }

   if (ret == STATUS_SUCCESS)
   {
      /* copy pages */
      for (i=0; i<bad_page; i++)
      {
         if (ret == STATUS_SUCCESS)
         {
            (void)MTD_Read(phy_block, i, tmp_data_buffer, spare);
            ret = MTD_Program(new_block, i, tmp_data_buffer, spare);
         }

         if (ret == STATUS_SUCCESS)
         {
            ret = MTD_WaitReady(new_block);
         }

         if (ret != STATUS_SUCCESS)
         {
            break;
         }
      }
   }

   if (ret == STATUS_SUCCESS)
   {
      /* put back bad block to free blocks with a higher ec to prevent
       * using soon. The block will be discarded only when erase failed.
       */
      INDEX_FreeBlock_Put(phy_block, phy_block_ec+STATIC_WL_THRESHOLD);
      INDEX_Update_AreaUpdate(log_block, new_block, new_ec);
      ret = INDEX_Update_Commit();
   }

   if (ret == STATUS_SUCCESS)
   {
      *new_phy_block = new_block;
      *new_phy_ec = new_ec;
   }
   else
   {
      *new_phy_block = INVALID_BLOCK;
      *new_phy_ec = INVALID_EC;
   }

   return ret;
}


static
UINT32 ubi_find_die_buffer(PHY_BLOCK block)
{
   UINT32   i;
   UINT32   die_index = block>>BLOCK_PER_PLANE_SHIFT;

   for (i=0; i<TOTAL_DIE_COUNT; i++)
   {
      if ((dice_hold[i].phy_block) != INVALID_BLOCK &&
          ((dice_hold[i].phy_block)>>BLOCK_PER_PLANE_SHIFT) == die_index)
      {
         break;
      }
   }

   ASSERT(i != TOTAL_DIE_COUNT);

   return i;
}


