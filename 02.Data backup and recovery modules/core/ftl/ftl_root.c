/*********************************************************
 * Module name: ftl_root.c
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
 *    FTL ROOT table: root node of PMT, and journal blocks
 *
 *********************************************************/


#include <core\inc\cmn.h>
#include <core\inc\ubi.h>

#include <sys\sys.h>

#include "ftl_inc.h"


ROOT  root_table;

static LOG_BLOCK  root_current_block;
static PAGE_OFF   root_current_page;
static UINT32     root_edition;


STATUS ROOT_Format()
{
   STATUS   ret;

   root_current_block = ROOT_BLOCK0;
   root_current_page = 0;
   root_edition = 0;
   root_table.backup_block_count = 0;

   ret = UBI_Erase(root_current_block, root_current_block);
   if (ret == STATUS_SUCCESS)
   {
      /* write to UBI */
      ret = ROOT_Commit();
   }

   return ret;
}



STATUS ROOT_Init()
{
   UINT32      i;
   SPARE       footprint;
   STATUS      ret = STATUS_SUCCESS;

   /* choose the latest block */
   ret = UBI_Read(ROOT_BLOCK0, 0, &root_table, NULL);
   if (ret == STATUS_SUCCESS)
   {
      root_edition = root_table.root_edition;
      root_current_block = ROOT_BLOCK0;

      ret = UBI_Read(ROOT_BLOCK1, 0, &root_table, NULL);
      if (ret == STATUS_SUCCESS)
      {
         if (root_edition > root_table.root_edition)
         {
            root_current_block = ROOT_BLOCK0;
         }
         else
         {
            root_current_block = ROOT_BLOCK1;
         }
      }
      else
      {
         root_current_block = ROOT_BLOCK0;
      }
   }
   else
   {
      root_current_block = ROOT_BLOCK1;
   }

   /* read out the latest valid page */
   for (i=0; i<PAGE_PER_PHY_BLOCK; i++)
   {
      ret = UBI_Read(root_current_block, i, &root_table, footprint);
      if (ret != STATUS_SUCCESS || footprint[0] == INVALID_INDEX)
      {
         break;
      }
   }

   if (i == PAGE_PER_PHY_BLOCK)
   {
      ASSERT(ret == STATUS_SUCCESS);
   }
   else
   {
      ASSERT(ret != STATUS_SUCCESS);
      ret = STATUS_SUCCESS;
   }

   if (ret == STATUS_SUCCESS)
   {
      /* read out the valid table */
      ret = UBI_Read(root_current_block,
                     i-1,
                     &root_table,
                     NULL);
   }

   if (ret == STATUS_SUCCESS)
   {
      root_current_page = i;
      root_edition = root_table.root_edition + 1;


      /* skip one page for possible PLR issue */
      (void)ROOT_Commit();
   }

   return ret;
}


STATUS ROOT_Commit()
{
   STATUS      ret = STATUS_SUCCESS;
   SPARE       footprint;
   LOG_BLOCK   next_block = INVALID_BLOCK;

   if (root_current_page == PAGE_PER_PHY_BLOCK)
   {
      /* write data in another block */
      next_block = root_current_block ^ 1;

      /* erase the block before write root */
      ret = UBI_Erase(next_block, next_block);
      if (ret == STATUS_SUCCESS)
      {
         root_current_page = 0;
         root_current_block = next_block;
      }
   }

   if (ret == STATUS_SUCCESS)
   {
      PM_NODE_SET_BLOCKPAGE(root_table.root_current_journal,
                            root_current_block, root_current_page);
      root_table.root_edition = root_edition++;

      footprint[0] = 0;

      /* write ROOT table in ram to UBI */
      ret = UBI_Write(root_current_block,
                      root_current_page,
                      &root_table,
                      footprint,
                      FALSE);
   }

   if (ret == STATUS_SUCCESS)
   {
      root_current_page ++;
   }

   return ret;
}


