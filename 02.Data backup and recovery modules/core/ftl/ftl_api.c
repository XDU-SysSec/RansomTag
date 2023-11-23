/*********************************************************
 * Module name: ftl_api.c
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
 *    FTL APIs.
 *
 *********************************************************/

#include <core\inc\cmn.h>
#include <core\inc\ftl.h>
#include <core\inc\ubi.h>
#include <core\inc\mtd.h>

#include <sys\sys.h>

#include "ftl_inc.h"

#include <recovery\recovery.h> 
static LOG_BLOCK lastblock;
static PAGE_OFF lastpage;
static PM_NODE_ADDR backupmft;
static PGADDR last_addr;

/* Advanced Page Mapping FTL:
 * - Block Dirty Table: LOG_BLOCK 0, cache all
 * - ROOT Table: LOG_BLOCK 1, cache all. point to journal blocks.
 * - Page Mapping Table: LOG_BLOCK 2~N, cache x pages with LRU algo.
 * - DATA Journal: commit
 * - Init: read BDT, ROOT, PMT, Journal info, ...
 * - Reclaim
 * - Meta Data Page: in last page in PMT blocks and data blocks.
 * - choose journal block on erase and write, according to die index
 *
 * TODO: advanced features:
 * - sanitizing
 * - bg erase
 * - check wp/trim, ...
 */

STATUS FTL_Format()
{
   STATUS ret;

   ret = UBI_Format();
   if (ret == STATUS_SUCCESS)
   {
      ret = UBI_Init();
   }

   if (ret == STATUS_SUCCESS)
   {
      ret = DATA_Format();
   }

   if (ret == STATUS_SUCCESS)
   {
      ret = HDI_Format();
   }

   if (ret == STATUS_SUCCESS)
   {
      ret = PMT_Format();
   }

   if (ret == STATUS_SUCCESS)
   {
      ret = BDT_Format();
   }

   if (ret == STATUS_SUCCESS)
   {
      ret = ROOT_Format();
   }

   return ret;
}

STATUS FTL_Init()
{
   STATUS ret;
   ret = UBI_Init();
   if (ret == STATUS_SUCCESS)
   {
      /* scan tables on UBI, and copy to RAM */
      ret = ROOT_Init();
   }

   if (ret == STATUS_SUCCESS)
   {
      ret = BDT_Init();
   }

   if (ret == STATUS_SUCCESS)
   {
      ret = PMT_Init();
   }

   if (ret == STATUS_SUCCESS)
   {
      ret = HDI_Init();
   }

   if (ret == STATUS_SUCCESS)
   {
      ret = DATA_Replay(root_table.hot_journal);
   }

   if (ret == STATUS_SUCCESS)
   {
      ret = DATA_Replay(root_table.cold_journal);
   }

   if (ret == STATUS_SUCCESS)
   {
      /* handle reclaim PLR: start reclaim again. Some data should
       * be written in the same place, so just rewrite same data in the
       * same page regardless this page is written or not.
       */

      /* check if hot journal blocks are full */
      if (DATA_IsFull(TRUE) == TRUE)
      {
         ret = DATA_Reclaim(TRUE);
         if (ret == STATUS_SUCCESS)
         {
            ret = DATA_Commit();
         }
      }

      /* check if cold journal blocks are full */
      if (DATA_IsFull(FALSE) == TRUE)
      {
         ret = DATA_Reclaim(FALSE);
         if (ret == STATUS_SUCCESS)
         {
            ret = DATA_Commit();
         }
      }
   }

   return ret;
}

STATUS FTL_Write(PGADDR addr, void *buffer, Boolean flag, Boolean mft)
{
   STATUS ret;
   BOOL is_hot = HDI_IsHotPage(addr);

   ret = DATA_Write(addr, buffer, is_hot, flag, mft);
   if (ret == STATUS_SUCCESS)
   {
      if (DATA_IsFull(is_hot) == TRUE)
      {
         ret = DATA_Reclaim(is_hot);
         if (ret == STATUS_SUCCESS)
         {
            ret = DATA_Commit();
         }
      }
   }

   return ret;
}

STATUS FTL_Read(PGADDR addr, void *buffer, Boolean flag, Boolean mft)
{
   LOG_BLOCK block;
   PAGE_OFF page;
   STATUS ret;
   SPARE spare;
   LOG_BLOCK old_block;
   LOG_BLOCK temp_block;

   ret = PMT_Search(addr, &block, &page);

   if (mft == FALSE && flag == FALSE && ret == STATUS_SUCCESS)
   {
      ret = UBI_Read(block, page, buffer, NULL);
      return ret;
   }

   //if (flag == FALSE && ((addr << SECTOR_PER_MPP_SHIFT) - pre_read_sector == 8))
   //{
   //   PRINTF("FTL_Read: sector %u is continous and has no tag, pre_read_sector=%u\n", addr << SECTOR_PER_MPP_SHIFT, pre_read_sector);
   //   flag == TRUE;
   //}

   else if (mft == TRUE)
   {
      // If accessing consecutive addresses, skip the following logic.
      if (addr == (last_addr + 1))
      {
         return UBI_Read(block, page, buffer, NULL);
      }
      last_addr = addr;

      // PRINTF("\nlogic addr:%d\nmft read start: Block:%d,Page:%d\n lastblock:%d,lastpage:%d\n", addr, block, page, lastblock, lastpage);
      // Read an address for the first time
      if (lastpage != page || lastblock != block)
      {
         lastblock = block;
         lastpage = page;
         ret = UBI_Read(block, page, buffer, spare);

         temp_block = PM_NODE_BLOCK(spare[2]);
         // if (temp_block > 2048 && spare[2]!=INVALID_PM_NODE)
         // {
         //    PRINTF("READ OUT SPARE 1ST temp_block :%d spare[2]:%d\n", temp_block, spare[2]);
         // }

         if (spare[2] == INVALID_PM_NODE)
         {
            // PRINTF("not READ OUT SPARE 1ST\n");
            PM_NODE_SET_BLOCKPAGE(backupmft, block, page);
         }
         else if (temp_block > 2048 || temp_block < 0)
         {
            PM_NODE_SET_BLOCKPAGE(backupmft, block, page);
            //  PRINTF("READ OUT SPARE but strange data%d\n,change to current addr %d \n", spare[2], backupmft);
         }
         else
         {
            // PRINTF("READ OUT SPARE 1ST\n");
            backupmft = spare[2];
         }
         // PRINTF("FIRST READ ,backupmft=%d\n ", backupmft);
         return ret;
      }
      // Read the same address for the nth time
      else if (lastpage == page && lastblock == block)
      {
         // if(backupmft == INVALID_PM_NODE){
         //    buffer = NULL;
         //    return ret;
         // }
         block = PM_NODE_BLOCK(backupmft);
         page = PM_NODE_PAGE(backupmft);
         // PRINTF("Nst in\n read start. backupmft = %d . Block:%d,Page:%d\n", backupmft, block, page);
         ret = UBI_Read(block, page, buffer, spare);

         block = PM_NODE_BLOCK(spare[2]);
         page = PM_NODE_PAGE(spare[2]);
         // if (block > 2048 && spare[2]!=INVALID_PM_NODE)
         // {
         //    PRINTF("READ OUT SPARE nST temp_block :%d spare[2]:%d\n", block, spare[2]);
         // }

         // PRINTF("READ OUT SPARE ADDR: Block:%d,Page:%d\n", block, page);
         if (block < 2049)
         {
            backupmft = spare[2];
            // PRINTF("change backupmft\n");
         }
         // PRINTF("spare[2]=%d ;backupmft update to %d .READ END\n", spare[2], backupmft);
         return ret;
      }
   }

   /* To read backup data, read the physical address stored in SPARE*/
   // if (flag == TRUE && mft == FALSE)
   else
   {
      //PRINTF("UBI_Read start:  sector=%u, block = %u,PAGE=%u\n", addr<<SECTOR_PER_MPP_SHIFT,block, page);
      if (block != INVALID_BLOCK && page != INVALID_PAGE)
      {
         ret = UBI_Read(block, page, NULL, spare);
         old_block = spare[2];

         if (old_block != INVALID_PM_NODE)
         {
            block = PM_NODE_BLOCK(old_block);
            page = PM_NODE_PAGE(old_block);
         }
      }
      //   pre_read_sector = addr << SECTOR_PER_MPP_SHIFT;
      //PRINTF("oldLogicAddr = %u, sector=%u, block=%u, page=%u\n", old_block, addr<<SECTOR_PER_MPP_SHIFT,block, page);
      if (ret == STATUS_SUCCESS)
      {
         ret = UBI_Read(block, page, buffer, NULL);
      }

      return ret;
   }
}

STATUS FTL_Trim(PGADDR start, PGADDR end)
{
   PGADDR addr;
   STATUS ret = STATUS_SUCCESS;

   for (addr = start; addr <= end; addr++)
   {
      ret = FTL_Write(addr, NULL, FALSE, FALSE);
      if (ret != STATUS_SUCCESS)
      {
         break;
      }
   }

   return ret;
}

STATUS FTL_SetWP(PGADDR laddr, BOOL enabled)
{
   return STATUS_FAILURE;
}

BOOL FTL_CheckWP(PGADDR laddr)
{
   return FALSE;
}

STATUS FTL_BgTasks()
{
   return STATUS_SUCCESS;
}

PGADDR FTL_Capacity()
{
   LOG_BLOCK block;

   block = UBI_Capacity;
   block -= JOURNAL_BLOCK_COUNT;               /* data hot journal */
   block -= JOURNAL_BLOCK_COUNT;               /* data cold journal */
   block -= JOURNAL_BLOCK_COUNT;               /* data reclaim journal */
   block -= PMT_BLOCK_COUNT;                   /* pmt blocks */
   block -= 2;                                 /* bdt blocks */
   block -= 2;                                 /* root blocks */
   block -= 2;                                 /* hdi reserved */
   block -= block / 100 * OVER_PROVISION_RATE; /* over provision */

   /* last page in every block is reserved for meta data collection */
   return block * (PAGE_PER_PHY_BLOCK - 1);
}

STATUS FTL_Flush()
{
   STATUS ret;

   ret = DATA_Commit();
   if (ret == STATUS_SUCCESS)
   {
      ret = UBI_Flush();
   }

   // #if (SIM_TEST == TRUE)
   //    if (ret == STATUS_SUCCESS)
   //    {
   //       /* just test the SWL in sim tests. The SWL should be
   //        * called in real HW platform in background, and make sure
   //        * the write buffer is flushed before SWL.
   //        */
   //       ret = UBI_SWL();
   //    }
   // #endif

   // if (ret == STATUS_SUCCESS)
   // {
   //    /* just test the SWL in sim tests. The SWL should be
   //     * called in real HW platform in background, and make sure
   //     * the write buffer is flushed before SWL.
   //     */
   //    ret = UBI_SWL();
   // }

   return ret;
}


// /*********************************************************
//  * Module name: ftl_api.c
//  *
//  * Copyright 2010, 2011. All Rights Reserved, Crane Chu.
//  *
//  * This file is part of OpenNFM.
//  *
//  * OpenNFM is free software: you can redistribute it and/or 
//  * modify it under the terms of the GNU General Public 
//  * License as published by the Free Software Foundation, 
//  * either version 3 of the License, or (at your option) any 
//  * later version.
//  * 
//  * OpenNFM is distributed in the hope that it will be useful,
//  * but WITHOUT ANY WARRANTY; without even the implied 
//  * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
//  * PURPOSE. See the GNU General Public License for more 
//  * details.
//  *
//  * You should have received a copy of the GNU General Public 
//  * License along with OpenNFM. If not, see 
//  * <http://www.gnu.org/licenses/>.
//  *
//  * First written on 2010-01-01 by cranechu@gmail.com
//  *
//  * Module Description:
//  *    FTL APIs.
//  *
//  *********************************************************/

// #include <core\inc\cmn.h>
// #include <core\inc\ftl.h>
// #include <core\inc\ubi.h>
// #include <core\inc\mtd.h>

// #include <sys\sys.h>

// #include "ftl_inc.h"

// #include <recovery\recovery.h> 
// static LOG_BLOCK lastblock;
// static PAGE_OFF lastpage;
// static PM_NODE_ADDR backupmft;
// static PGADDR last_addr;

// /* Advanced Page Mapping FTL:
//  * - Block Dirty Table: LOG_BLOCK 0, cache all
//  * - ROOT Table: LOG_BLOCK 1, cache all. point to journal blocks.
//  * - Page Mapping Table: LOG_BLOCK 2~N, cache x pages with LRU algo.
//  * - DATA Journal: commit
//  * - Init: read BDT, ROOT, PMT, Journal info, ...
//  * - Reclaim
//  * - Meta Data Page: in last page in PMT blocks and data blocks.
//  * - choose journal block on erase and write, according to die index
//  *
//  * TODO: advanced features:
//  * - sanitizing
//  * - bg erase
//  * - check wp/trim, ...
//  */

// STATUS FTL_Format()
// {
//    STATUS ret;

//    ret = UBI_Format();
//    if (ret == STATUS_SUCCESS)
//    {
//       ret = UBI_Init();
//    }

//    if (ret == STATUS_SUCCESS)
//    {
//       ret = DATA_Format();
//    }

//    if (ret == STATUS_SUCCESS)
//    {
//       ret = HDI_Format();
//    }

//    if (ret == STATUS_SUCCESS)
//    {
//       ret = PMT_Format();
//    }

//    if (ret == STATUS_SUCCESS)
//    {
//       ret = BDT_Format();
//    }

//    if (ret == STATUS_SUCCESS)
//    {
//       ret = ROOT_Format();
//    }

//    return ret;
// }

// STATUS FTL_Init()
// {
//    STATUS ret;
//    ret = UBI_Init();
//    if (ret == STATUS_SUCCESS)
//    {
//       /* scan tables on UBI, and copy to RAM */
//       ret = ROOT_Init();
//    }

//    if (ret == STATUS_SUCCESS)
//    {
//       ret = BDT_Init();
//    }

//    if (ret == STATUS_SUCCESS)
//    {
//       ret = PMT_Init();
//    }

//    if (ret == STATUS_SUCCESS)
//    {
//       ret = HDI_Init();
//    }

//    if (ret == STATUS_SUCCESS)
//    {
//       ret = DATA_Replay(root_table.hot_journal);
//    }

//    if (ret == STATUS_SUCCESS)
//    {
//       ret = DATA_Replay(root_table.cold_journal);
//    }

//    if (ret == STATUS_SUCCESS)
//    {
//       /* handle reclaim PLR: start reclaim again. Some data should
//        * be written in the same place, so just rewrite same data in the
//        * same page regardless this page is written or not.
//        */

//       /* check if hot journal blocks are full */
//       if (DATA_IsFull(TRUE) == TRUE)
//       {
//          ret = DATA_Reclaim(TRUE);
//          if (ret == STATUS_SUCCESS)
//          {
//             ret = DATA_Commit();
//          }
//       }

//       /* check if cold journal blocks are full */
//       if (DATA_IsFull(FALSE) == TRUE)
//       {
//          ret = DATA_Reclaim(FALSE);
//          if (ret == STATUS_SUCCESS)
//          {
//             ret = DATA_Commit();
//          }
//       }
//    }

//    return ret;
// }

// STATUS FTL_Write(PGADDR addr, void *buffer, Boolean flag, Boolean mft)
// {
//    STATUS ret;
//    BOOL is_hot = HDI_IsHotPage(addr);

//    ret = DATA_Write(addr, buffer, is_hot, flag, mft);
//    if (ret == STATUS_SUCCESS)
//    {
//       if (DATA_IsFull(is_hot) == TRUE)
//       {
//          ret = DATA_Reclaim(is_hot);
//          if (ret == STATUS_SUCCESS)
//          {
//             ret = DATA_Commit();
//          }
//       }
//    }

//    return ret;
// }

// STATUS FTL_Read(PGADDR addr, void *buffer, Boolean flag, Boolean mft)
// {
//    LOG_BLOCK block;
//    PAGE_OFF page;
//    STATUS ret;
//    SPARE spare;
//    LOG_BLOCK old_block;
//    LOG_BLOCK temp_block;

//    ret = PMT_Search(addr, &block, &page);

//    //if (flag == FALSE && ((addr << SECTOR_PER_MPP_SHIFT) - pre_read_sector == 8))
//    //{
//    //   PRINTF("FTL_Read: sector %u is continous and has no tag, pre_read_sector=%u\n", addr << SECTOR_PER_MPP_SHIFT, pre_read_sector);
//    //   flag == TRUE;
//    //}

//    if (mft == TRUE)
//    {

//       // if (block == INVALID_BLOCK || page == INVALID_PAGE)
//       // {
//       //    ret = UBI_Read(block, page, buffer, NULL);
//       //    return ret;
//       // }
//       // if (block > 2048 && spare[2] != INVALID_PM_NODE)
//       // {
//       //    PRINTF("IN BLOCK:%d \n", block);
//       // }
//       // 如果访问的是连续地址，跳过以下逻辑。
//       if (addr == (last_addr + 1))
//       {
//          return UBI_Read(block, page, buffer, NULL);
//       }
//       last_addr = addr;

//       // PRINTF("\nlogic addr:%d\nmft read start: Block:%d,Page:%d\n lastblock:%d,lastpage:%d\n", addr, block, page, lastblock, lastpage);
//       // 第1次读一个地址
//       if (lastpage != page || lastblock != block)
//       {
//          // PRINTF("1st In \n");
//          lastblock = block;
//          lastpage = page;
//          ret = UBI_Read(block, page, buffer, spare);

//          temp_block = PM_NODE_BLOCK(spare[2]);
//          // if (temp_block > 2048 && spare[2]!=INVALID_PM_NODE)
//          // {
//          //    PRINTF("READ OUT SPARE 1ST temp_block :%d spare[2]:%d\n", temp_block, spare[2]);
//          // }

//          if (spare[2] == INVALID_PM_NODE)
//          {
//             // PRINTF("not READ OUT SPARE 1ST\n");
//             PM_NODE_SET_BLOCKPAGE(backupmft, block, page);
//          }
//          else if (temp_block > 2048 || temp_block < 0)
//          {
//             PM_NODE_SET_BLOCKPAGE(backupmft, block, page);
//             //  PRINTF("READ OUT SPARE but strange data%d\n,change to current addr %d \n", spare[2], backupmft);
//          }
//          else
//          {
//             // PRINTF("READ OUT SPARE 1ST\n");
//             backupmft = spare[2];
//          }
//          // PRINTF("FIRST READ ,backupmft=%d\n ", backupmft);
//          return ret;
//       }
//       // 第n次读同一个地址
//       else if (lastpage == page && lastblock == block)
//       {
//          // if(backupmft == INVALID_PM_NODE){
//          //    buffer = NULL;
//          //    return ret;
//          // }
//          block = PM_NODE_BLOCK(backupmft);
//          page = PM_NODE_PAGE(backupmft);
//          // PRINTF("Nst in\n read start. backupmft = %d . Block:%d,Page:%d\n", backupmft, block, page);
//          ret = UBI_Read(block, page, buffer, spare);

//          block = PM_NODE_BLOCK(spare[2]);
//          page = PM_NODE_PAGE(spare[2]);
//          // if (block > 2048 && spare[2]!=INVALID_PM_NODE)
//          // {
//          //    PRINTF("READ OUT SPARE nST temp_block :%d spare[2]:%d\n", block, spare[2]);
//          // }

//          // PRINTF("READ OUT SPARE ADDR: Block:%d,Page:%d\n", block, page);
//          if (block < 2049)
//          {
//             backupmft = spare[2];
//             // PRINTF("change backupmft\n");
//          }
//          // PRINTF("spare[2]=%d ;backupmft update to %d .READ END\n", spare[2], backupmft);
//          return ret;
//       }
//    }

//    /* 如果要读备份数据，读SPARE中存的物理地址 */
//    if (flag == TRUE && mft == FALSE)
//    {
//       //PRINTF("UBI_Read start:  sector=%u, block = %u,PAGE=%u\n", addr<<SECTOR_PER_MPP_SHIFT,block, page);
//       if (block != INVALID_BLOCK && page != INVALID_PAGE)
//       {
//          ret = UBI_Read(block, page, NULL, spare);
//          old_block = spare[2];

//          if (old_block != INVALID_PM_NODE)
//          {
//             block = PM_NODE_BLOCK(old_block);
//             page = PM_NODE_PAGE(old_block);
//          }
//       }
//       //   pre_read_sector = addr << SECTOR_PER_MPP_SHIFT;
//       //PRINTF("oldLogicAddr = %u, sector=%u, block=%u, page=%u\n", old_block, addr<<SECTOR_PER_MPP_SHIFT,block, page);
//    }

//    if (ret == STATUS_SUCCESS)
//    {
//       ret = UBI_Read(block, page, buffer, NULL);
//    }

//    return ret;
// }

// STATUS FTL_Trim(PGADDR start, PGADDR end)
// {
//    PGADDR addr;
//    STATUS ret = STATUS_SUCCESS;

//    for (addr = start; addr <= end; addr++)
//    {
//       ret = FTL_Write(addr, NULL, FALSE, FALSE);
//       if (ret != STATUS_SUCCESS)
//       {
//          break;
//       }
//    }

//    return ret;
// }

// STATUS FTL_SetWP(PGADDR laddr, BOOL enabled)
// {
//    return STATUS_FAILURE;
// }

// BOOL FTL_CheckWP(PGADDR laddr)
// {
//    return FALSE;
// }

// STATUS FTL_BgTasks()
// {
//    return STATUS_SUCCESS;
// }

// PGADDR FTL_Capacity()
// {
//    LOG_BLOCK block;

//    block = UBI_Capacity;
//    block -= JOURNAL_BLOCK_COUNT;               /* data hot journal */
//    block -= JOURNAL_BLOCK_COUNT;               /* data cold journal */
//    block -= JOURNAL_BLOCK_COUNT;               /* data reclaim journal */
//    block -= PMT_BLOCK_COUNT;                   /* pmt blocks */
//    block -= 2;                                 /* bdt blocks */
//    block -= 2;                                 /* root blocks */
//    block -= 2;                                 /* hdi reserved */
//    block -= block / 100 * OVER_PROVISION_RATE; /* over provision */

//    /* last page in every block is reserved for meta data collection */
//    return block * (PAGE_PER_PHY_BLOCK - 1);
// }

// STATUS FTL_Flush()
// {
//    STATUS ret;

//    ret = DATA_Commit();
//    if (ret == STATUS_SUCCESS)
//    {
//       ret = UBI_Flush();
//    }

//    // #if (SIM_TEST == TRUE)
//    //    if (ret == STATUS_SUCCESS)
//    //    {
//    //       /* just test the SWL in sim tests. The SWL should be
//    //        * called in real HW platform in background, and make sure
//    //        * the write buffer is flushed before SWL.
//    //        */
//    //       ret = UBI_SWL();
//    //    }
//    // #endif

//    // if (ret == STATUS_SUCCESS)
//    // {
//    //    /* just test the SWL in sim tests. The SWL should be
//    //     * called in real HW platform in background, and make sure
//    //     * the write buffer is flushed before SWL.
//    //     */
//    //    ret = UBI_SWL();
//    // }

//    return ret;
// }
