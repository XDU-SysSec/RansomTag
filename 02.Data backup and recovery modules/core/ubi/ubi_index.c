/*********************************************************
 * Module name: ubi_index.c
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
 *    Manage index table in ram and nand.
 *
 *********************************************************/


#include <core\inc\cmn.h>
#include <core\inc\mtd.h>

#include <sys\sys.h>

#include "ubi_inc.h"


/*
 * ANCHOR table is not managed by FTL/TABLE module. It is self-PLR-safe.
 *
 * To achieve the Static Wear Leveling (SWL), the EC of every PHY_BLOCK
 * is traced in Erase Count Table (ECT), which is located in the following
 * sector of above tables. SWL works as a background task. A block in a area,
 * which has the minimal EC, is selected, and switched with the first
 * block in FBT, only when the difference of their ECs is larger than a
 * predefined threhold. In this way, the static blocks can be found and used.
 *
 * An index table has 4 sectors. The 1st sector is ZIT, the 2nd is ECT
 * of ZIT; the 3rd one has FBT, IBT, CBT, RBT, RFT, and the 4th one is the ECT.
 */


static
STATUS index_update();


INDEX_TABLE index_table;

/* current index table block, and its erase count */
static PHY_BLOCK     index_block;
static ERASE_COUNT   index_block_ec;
static PAGE_OFF      index_next_page;

static BOOL          is_updating_area = FALSE;


PHY_BLOCK INDEX_Format(PHY_BLOCK total_block, PHY_BLOCK fmt_current_block)
{
   UINT32      i;
   UINT32      free_block_index = 0;
   PHY_BLOCK   index_block = INVALID_BLOCK;
   STATUS      ret = STATUS_SUCCESS;

   /* clear plr info */
   index_table.area_update_plr.area_updating_logical_block = INVALID_BLOCK;
   index_table.area_update_plr.area_updating_physical_block = INVALID_BLOCK;
   index_table.area_update_plr.area_updating_block_ec = INVALID_EC;

   /* free block table: all remaining block */
   for(i=0; fmt_current_block<total_block; fmt_current_block++,i++)
   {
      /* log good block, discard bad block */
      if (ANCHOR_IsBadBlock(fmt_current_block) == FALSE)
      {
         ret = MTD_Erase(fmt_current_block);
         if (ret != STATUS_SUCCESS)
         {
            /* mark bad block */
            ANCHOR_LogBadBlock(fmt_current_block);
         }
      }
      else
      {
         ret = STATUS_BADBLOCK;
      }

      if (ret == STATUS_SUCCESS)
      {
         if (index_block == INVALID_BLOCK)
         {
            /* the first free block should be reserved for index block */
            index_block = fmt_current_block;
         }
         else
         {
            /* the reserved free block in block index table should be large
             * enough to hold the 2% reserved free block of total block count.
             */
            ASSERT(free_block_index < FREE_BLOCK_COUNT);
            index_table.free_block_table[free_block_index] = fmt_current_block;
            index_table.free_block_ect[free_block_index] = 0;
            free_block_index ++;
         }
      }
   }

   /* fill all remaining free block table as invalid block */
   for (i=free_block_index; i<FREE_BLOCK_COUNT; i++)
   {
      index_table.free_block_table[i] = INVALID_BLOCK;
      index_table.free_block_ect[i] = INVALID_EC;
   }

   ASSERT(sizeof(index_table) == MPP_SIZE);

   /* write index table, with EC sectors */
   if (index_block == INVALID_BLOCK)
   {
      ASSERT(ret != STATUS_SUCCESS);
   }

   /* write area table, with EC sector, check and erase block first */
   if (ret == STATUS_SUCCESS)
   {
      if (ANCHOR_IsBadBlock(index_block) == TRUE)
      {
         ret = STATUS_BADBLOCK;
      }
   }

   if (ret == STATUS_SUCCESS)
   {
      ret = MTD_Erase(index_block);
   }

   if (ret == STATUS_SUCCESS)
   {
      ret = TABLE_Write(index_block, 0, &index_table);
   }

   if (ret == STATUS_SUCCESS)
   {
      /* setup index block info in cfg table */
      anchor_table.index_new_block = index_block;
      anchor_table.index_new_ec = 0;
      anchor_table.index_old_block = INVALID_BLOCK;
      anchor_table.index_old_ec = INVALID_EC;
   }
   else
   {
      /* mark bad block, pick another block for index. */
      ANCHOR_LogBadBlock(index_block);

      fmt_current_block = INVALID_BLOCK;
   }

   return fmt_current_block;
}


STATUS INDEX_Init(PHY_BLOCK*      logical_block,
                  PHY_BLOCK*      origin_block,
                  ERASE_COUNT*    block_ec)
{
   PAGE_OFF    page_offset = INVALID_OFFSET;
   STATUS      ret = STATUS_FAILURE;

   is_updating_area = FALSE;

   /* PLR of index block reclaim: try to read new block first */
   ASSERT(anchor_table.index_new_block != INVALID_BLOCK);
   ret = TABLE_Read(anchor_table.index_new_block, &page_offset, &index_table);
   if (ret == STATUS_SUCCESS)
   {
      /* the new block has valid index table */
      index_block = anchor_table.index_new_block;
      index_block_ec = anchor_table.index_new_ec;
      index_next_page = page_offset + 1;

      /* this page may be written before PL, just write it to overwrite it */
      (void)index_update();
   }
   else
   {
      /* can not get correct data from new block, read in old block for PLR */
      ASSERT(anchor_table.index_old_block != INVALID_BLOCK);
      ret = TABLE_Read(anchor_table.index_old_block,
                       &page_offset,
                       &index_table);
      if (ret == STATUS_SUCCESS)
      {
         /* finish the reclaim before PL */
         index_block = anchor_table.index_old_block;
         index_block_ec = anchor_table.index_old_ec;
         index_next_page = PAGE_PER_PHY_BLOCK;

         /* update index table to new block */
         ret = index_update();
      }
   }

   /* set up the area plr info */
   *logical_block = index_table.area_update_plr.area_updating_logical_block;
   *origin_block = index_table.area_update_plr.area_updating_physical_block;
   *block_ec = index_table.area_update_plr.area_updating_block_ec;

   return ret;
}


void INDEX_Update_AreaReclaim(AREA area, PHY_BLOCK new_block, ERASE_COUNT nec)
{
   /* release old area block */
   INDEX_FreeBlock_Put(index_table.area_index_table[area],
                       index_table.area_index_ect[area]);

   /* setup new area block */
   index_table.area_index_table[area] = new_block;
   index_table.area_index_ect[area] = nec;
}


void INDEX_Update_AreaUpdate(LOG_BLOCK     logical_block,
                             PHY_BLOCK     physical_block,
                             ERASE_COUNT   block_ec)
{
   ASSERT(physical_block != INVALID_BLOCK && block_ec != INVALID_EC);

   index_table.area_update_plr.area_updating_logical_block = logical_block;
   index_table.area_update_plr.area_updating_physical_block = physical_block;
   index_table.area_update_plr.area_updating_block_ec = block_ec;

   is_updating_area = TRUE;
}


/* update index table, and area table if necessary */
STATUS INDEX_Update_Commit()
{
   AREA           area;
   BOOL           area_reclaim = FALSE;
   STATUS         ret = STATUS_SUCCESS;
   PHY_BLOCK      new_area_block = INVALID_BLOCK;
   PHY_BLOCK      updating_logical_block;
   PHY_BLOCK      updating_physical_block;
   ERASE_COUNT    updating_block_ec;
   ERASE_COUNT    new_area_ec;

   do
   {
      if (is_updating_area == TRUE)
      {
         updating_logical_block =
               index_table.area_update_plr.area_updating_logical_block;
         updating_physical_block =
               index_table.area_update_plr.area_updating_physical_block;
         updating_block_ec =
               index_table.area_update_plr.area_updating_block_ec;

         /* update area table, the 2nd-level index table */
         if (AREA_IsFull(updating_logical_block) == TRUE)
         {
            /* update area table in another new area table block.
             * the new block is only valid after writing index table, so no PLR
             * issue between writing these two blocks.
             */
            area = AREA_INDEX(updating_logical_block);
            ret = INDEX_FreeBlock_Get(index_table.area_index_table[area],
                                      &new_area_block,
                                      &new_area_ec);
            if (ret == STATUS_SUCCESS)
            {

               /* update info of area index table */
               INDEX_Update_AreaReclaim(area, new_area_block, new_area_ec);

               area_reclaim = TRUE;
            }
         }
         else
         {
            /* update index table with area update info for PLR.
             * PLR: get the area update info from index table, and check the
             *      area table if need to update due to PL.
             */
            ret = index_update();
         }

         if (ret == STATUS_SUCCESS)
         {
            /* write area block */
            ret = AREA_Update(updating_logical_block,
                              updating_physical_block,
                              updating_block_ec);

            if (ret == STATUS_SUCCESS && area_reclaim == TRUE)
            {
               /* update index later than area, if area block reclaimed */
                ret = index_update();
            }
         }

         if (ret == STATUS_SUCCESS)
         {
            /* CLEAR the area update flag */
            is_updating_area = FALSE;
         }
      }
      else
      {
         /* only update index table */
         ret = index_update();
      }
   } while (ret == STATUS_BADBLOCK);

   return ret;
}


void INDEX_FreeBlock_GetMaxECBlock(PHY_BLOCK*     physical_block,
                                   ERASE_COUNT*   block_ec)
{
   UINT32   i;

   for (i=FREE_BLOCK_COUNT-1; i>0; i--)
   {
      if (index_table.free_block_ect[i] != INVALID_EC)
      {
         *physical_block = index_table.free_block_table[i];
         *block_ec = index_table.free_block_ect[i];
         break;
      }
   }

   return;
}


void INDEX_FreeBlock_SwapMaxECBlock(PHY_BLOCK      min_ec_block,
                                    ERASE_COUNT    min_ec)
{
   UINT32   i;

   /* swap for SWL:
    * - find the max ec good block in free block,
    * - re-sort and inset the min ec block
    */

   /* GET the max ec good block in the sorted free block */
   for (i=FREE_BLOCK_COUNT-1; i>0; i--)
   {
      if (index_table.free_block_ect[i] != INVALID_EC)
      {
         break;
      }
   }

   if (index_table.free_block_ect[i] != INVALID_EC &&
       index_table.free_block_ect[i] > min_ec &&
       index_table.free_block_ect[i]-min_ec > STATIC_WL_THRESHOLD)
   {
      /* insert the min ec block to sorted free block table,
       * continue to scan the free block table to head.
       */
      for (i=i-1; i>0; i--)
      {
         if (index_table.free_block_ect[i] > min_ec)
         {
            index_table.free_block_table[i+1] =
                           index_table.free_block_table[i];
            index_table.free_block_ect[i+1] =
                           index_table.free_block_ect[i];
         }
         else
         {
            /* insert the min ec block in current position */
            index_table.free_block_table[i+1] = min_ec_block;
            index_table.free_block_ect[i+1] = min_ec;

            break;
         }
      }

      /* an special case due to i is unsigned char */
      if (i == 0)
      {
         if (index_table.free_block_ect[0] > min_ec)
         {
            index_table.free_block_table[1] =
                           index_table.free_block_table[0];
            index_table.free_block_ect[1] =
                           index_table.free_block_ect[0];

            index_table.free_block_table[0] = min_ec_block;
            index_table.free_block_ect[0] = min_ec;
         }
         else
         {
            /* insert the min ec block in current position */
            index_table.free_block_table[1] = min_ec_block;
            index_table.free_block_ect[1] = min_ec;
         }
      }
   }
}


/* the EC of the free block in FBT:
 * MSB: set when read/program fail, reset if erased successfully
 * when erasing a block in FBT failed, just discard it, and get a new block
 * from IBT! If none in IBT, discard the bad, and get another in FBT.
 */
STATUS INDEX_FreeBlock_Get(DIE_INDEX die, PHY_BLOCK* block, ERASE_COUNT* ec)
{
   STATUS   ret;
   UINT32   i;

   die = die % TOTAL_DIE_COUNT;

   do
   {
      for (i=0; i<FREE_BLOCK_COUNT; i++)
      {
         /* get new block at the head of free block table,
          * whose ec is min, and in the same die.
          */
         if (index_table.free_block_table[i] == INVALID_BLOCK)
         {
            i=FREE_BLOCK_COUNT;
            break;
         }
         else if ((index_table.free_block_table[i]%TOTAL_DIE_COUNT) == die)
         {
            *block = index_table.free_block_table[i];
            break;
         }
      }

      if (i == FREE_BLOCK_COUNT)
      {
         /* can not find the block in the same die, get another */
         i = 0;
         *block = index_table.free_block_table[0];
      }

      if (*block != INVALID_BLOCK)
      {
         ASSERT(i < FREE_BLOCK_COUNT);

         /* no background erase. Erase block before using it. Most of erase
          * would happen in background reclaim.
          */
         ret = MTD_Erase(*block);
      }
      else
      {
         ret = STATUS_TOOMANY_BADBLOCK;
      }

      if (ret != STATUS_SUCCESS)
      {
         /* discard and log the bad block */
         ANCHOR_LogBadBlock(*block);
      }

      if (ret == STATUS_SUCCESS)
      {
         /* increase erase count of the new free block */
         *ec = index_table.free_block_ect[i] + 1;
      }

      if (ret != STATUS_TOOMANY_BADBLOCK)
      {
         /* move forward all other blocks, discard current block in FBT */
         for (i=i; i<FREE_BLOCK_COUNT-1; i++)
         {
            index_table.free_block_table[i] = index_table.free_block_table[i+1];
            index_table.free_block_ect[i] = index_table.free_block_ect[i+1];
         }

         /* fill the last free block entry with invalid block info */
         ASSERT(i == FREE_BLOCK_COUNT-1);
         index_table.free_block_table[i] = INVALID_BLOCK;
         index_table.free_block_ect[i] = INVALID_EC;
      }

      /* if not get a free block, and there is good block left (e.g. not
       * too many bad block generated.), try again.
       */
   } while (ret != STATUS_SUCCESS &&
            ret != STATUS_TOOMANY_BADBLOCK &&
            ret != STATUS_SimulatedPowerLoss);

   return ret;
}


void INDEX_FreeBlock_Put(PHY_BLOCK dirty_block, ERASE_COUNT dirty_block_ec)
{
   UINT32   i;

   /* the last item of FBT will be discarded to insert the new free block */
   for (i=FREE_BLOCK_COUNT-2; i>0; i--)
   {
      /* search the max ec block less than dirty_block_ec */
      if (index_table.free_block_ect[i] > dirty_block_ec)
      {
         index_table.free_block_table[i+1] =
                        index_table.free_block_table[i];
         index_table.free_block_ect[i+1] =
                        index_table.free_block_ect[i];
      }
      else
      {
         break;
      }
   }

   /* insert new free block at the position of i+1, or at the beginning */
   if (i > 0)
   {
      index_table.free_block_table[i+1] = dirty_block;
      index_table.free_block_ect[i+1] = dirty_block_ec;
   }
   else
   {
      ASSERT(i == 0);

      if (index_table.free_block_ect[0] > dirty_block_ec)
      {
         index_table.free_block_table[1] = index_table.free_block_table[0];
         index_table.free_block_ect[1] = index_table.free_block_ect[0];

         index_table.free_block_table[0] = dirty_block;
         index_table.free_block_ect[0] = dirty_block_ec;
      }
      else
      {
         index_table.free_block_table[1] = dirty_block;
         index_table.free_block_ect[1] = dirty_block_ec;
      }
   }
}


static
STATUS index_update()
{
   ERASE_COUNT    ec;
   PHY_BLOCK      free_block;
   STATUS         ret = STATUS_SUCCESS;

   /* Reclaim and PLR:
    *
    * If area block is updated successfully, its data is valid, and
    * mismatch with index table. So, a PLR info is required in index table.
    *
    * Process:
    * - find the new block, and log PLR info to cfg table.
    *   PLR: find the PLR info in cfg table, try to read
    *        index table in new block. If failed, read the table
    *        still from the old block. Omit the latest reclaim
    *        log, and start another reclaim. The new block is still
    *        a free block in FBT, neither used, nor lost. Only need
    *        to do another erase before get it, and the EC is not
    *        updated, but it is not a big issue due to few PL happened.
    *        We just guartee the integrity and functionality of system.
    * - write index table to new block, with the updated free table
    *   and its ECT.
    *   PLR: nothing to do. The new block is integrity now.
    */
   if (index_next_page == PAGE_PER_PHY_BLOCK)
   {
      /* log reclaim in cfg table */
      ret = INDEX_FreeBlock_Get(index_block, &free_block, &ec);
      if (ret == STATUS_SUCCESS)
      {
         ANCHOR_IndexReclaim(index_block, index_block_ec, free_block, ec);
         ret = ANCHOR_Update();
      }

      if (ret == STATUS_SUCCESS)
      {
         /* release the current index block */
         INDEX_FreeBlock_Put(index_block, index_block_ec);

         /* update index block to new free block */
         index_block = free_block;
         index_next_page = 0;
         index_block_ec = ec;
      }
   }

   if (ret == STATUS_SUCCESS)
   {
      /* one page write in index block, NO PLR issue */
      ret = TABLE_Write(index_block, index_next_page, &index_table);
   }

   if (ret == STATUS_SUCCESS)
   {
      index_next_page ++;
   }
   else if (ret == STATUS_BADBLOCK)
   {
      /* bad block, set the next page offset to toggle index block reclaim. */
      index_next_page = PAGE_PER_PHY_BLOCK;
      /* discard the bad block */
      index_block_ec = INVALID_EC;
   }

   return ret;
}


