/*********************************************************
 * Module name: ftl_hdi.c
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
 *    Hot Data Identification
 *
 *********************************************************/


#include <core\inc\cmn.h>
#include <core\inc\ubi.h>

#include <sys\sys.h>

#include "ftl_inc.h"


static UINT8      hdi_hash_table[MPP_SIZE];
static PAGE_OFF   hdi_current_page;
static LOG_BLOCK  hdi_current_block;


/* TODO: choose other hash functions and parameters to optimize HDI */
#define HDI_FUNCTION(a, i)          (((a)>>((i)*3))&(MPP_SIZE-1))
#define HDI_FUNC_COUNT              (4)
#define HDI_HOT_DATA_THERSHOLD      (0x60)
#define HDI_COLDDOWN_DELAY          (0x1000)


STATUS HDI_Format()
{
   UINT32   i;
   STATUS   ret;

   for (i=0; i<MPP_SIZE; i++)
   {
      hdi_hash_table[i] = 0;
   }

   hdi_current_block = HDI_BLOCK0;
   hdi_current_page = 0;

   ret = UBI_Erase(hdi_current_block, hdi_current_block);
   if (ret == STATUS_SUCCESS)
   {
      /* write to UBI */
      ret = HDI_Commit();
   }

   return ret;
}


STATUS HDI_Init()
{
   UINT32      i;
   STATUS      ret = STATUS_SUCCESS;

   hdi_current_block = PM_NODE_BLOCK(root_table.hdi_current_journal);
   hdi_current_page = PM_NODE_PAGE(root_table.hdi_current_journal);

   /* read out the valid page of table */
   ret = UBI_Read(hdi_current_block, hdi_current_page, hdi_hash_table, NULL);
   ASSERT(ret == STATUS_SUCCESS);

   /* scan for the first erased page */
   for (i=hdi_current_page+1; i<PAGE_PER_PHY_BLOCK; i++)
   {
      ret = UBI_Read(hdi_current_block, i, NULL, NULL);
      if (ret != STATUS_SUCCESS)
      {
         /* this page is not programmed */
         hdi_current_page = i;
         ret = STATUS_SUCCESS;
         break;
      }
   }

   if (i == PAGE_PER_PHY_BLOCK)
   {
      ASSERT(ret == STATUS_SUCCESS);

      /* current block is full */
      hdi_current_page = PAGE_PER_PHY_BLOCK;
   }

   if (ret == STATUS_SUCCESS)
   {
      /* skip one page for possible PLR issue */
      (void)BDT_Commit();
   }

   return ret;
}


BOOL HDI_IsHotPage(PGADDR addr)
{
   static UINT32  count = 0;
   UINT32         i;
   UINT8*         hot_value;
   BOOL           ret = TRUE;

   /* increase all hash slots when writing the page */
   for (i=0; i<HDI_FUNC_COUNT; i++)
   {
      hot_value = &hdi_hash_table[HDI_FUNCTION(addr, i)];

      if (*hot_value != MAX_UINT8)
      {
         (*hot_value) ++;
      }

      if (*hot_value < HDI_HOT_DATA_THERSHOLD)
      {
         ret = FALSE;
      }
   }

   /* cold down the hash table after every XXX times of access */
   if (count++ == HDI_COLDDOWN_DELAY)
   {
      for (i=0; i<MPP_SIZE; i++)
      {
         hdi_hash_table[i] >>= 1;
      }

      count = 0;
   }

   return ret;
}


STATUS HDI_Commit()
{
   STATUS      ret = STATUS_SUCCESS;
   LOG_BLOCK   next_block = INVALID_BLOCK;

   if (hdi_current_page == PAGE_PER_PHY_BLOCK)
   {
      /* write data in another block */
      next_block = hdi_current_block ^ 1;

      /* erase the block before write bdt */
      ret = UBI_Erase(next_block, next_block);
      if (ret == STATUS_SUCCESS)
      {
         hdi_current_page = 0;
         hdi_current_block = next_block;
      }
   }

   /* write BDT in ram to UBI */
   if (ret == STATUS_SUCCESS)
   {
      ret = UBI_Write(hdi_current_block,
                      hdi_current_page,
                      hdi_hash_table,
                      NULL,
                      FALSE);
   }

   if (ret == STATUS_SUCCESS)
   {
      PM_NODE_SET_BLOCKPAGE(root_table.hdi_current_journal,
                            hdi_current_block, hdi_current_page);
      hdi_current_page ++;
   }

   return ret;
}


