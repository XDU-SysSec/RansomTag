/*********************************************************
 * Module name: ubi_area.c
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
 *    Manage area table in ram and nand.
 *
 *********************************************************/


#include <core\inc\cmn.h>
#include <core\inc\mtd.h>

#include <sys\sys.h>

#include "ubi_inc.h"


typedef struct {
   PHY_BLOCK   physical_block;
   ERASE_COUNT physical_block_ec;
} AREA_BLOCK;

/* in var area */
static PAGE_OFF      area_offset_table[AREA_TABLE_SIZE];
static AREA_BLOCK    cached_area_table[CFG_PHY_BLOCK_PER_AREA];
static AREA          cached_area_number;


STATUS AREA_Init(AREA area_index)
{
   STATUS   ret;

   /* check the type defination */
   ASSERT(sizeof(PHY_BLOCK) == sizeof(ERASE_COUNT));
   ASSERT(sizeof(PHY_BLOCK) == sizeof(PAGE_OFF));

   /* find the offset of the area table */
   cached_area_number = INVALID_AREA;
   area_offset_table[area_index] = INVALID_PAGE;
   ret = AREA_Read(area_index);

   return ret;
}


PHY_BLOCK AREA_Format(PHY_BLOCK     block_count,
                      PHY_BLOCK     fmt_current_block,
                      AREA          area)
{
   UINT32   i;
   STATUS   ret = STATUS_SUCCESS;

   memset(&cached_area_table[0], 0xff, MPP_SIZE);

   /* check blocks, log bad block in anchor_table. Program AREA table */
   for (i=0; i<block_count; fmt_current_block++)
   {
      if (fmt_current_block < CFG_LOG_BLOCK_COUNT)
      {
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
      }
      else
      {
         break;
      }

      if (ret == STATUS_SUCCESS)
      {
         /* good block, trace in zone table */
         cached_area_table[i].physical_block = fmt_current_block;
         cached_area_table[i].physical_block_ec = 0;

         i ++;
      }
   }
  // block_count=512,CFG_AREA=128 没进去
   for (i=block_count; i<CFG_PHY_BLOCK_PER_AREA; i++)
   {
      /* set all remaining invalid */
      cached_area_table[i].physical_block = INVALID_BLOCK;
      cached_area_table[i].physical_block_ec = INVALID_EC;
   }

   /* write area table, with EC sector, check and erase block first */
   if (ret == STATUS_SUCCESS)
   {
      while (ANCHOR_IsBadBlock(fmt_current_block) == TRUE)
      {
         fmt_current_block ++;
      }

      ret = MTD_Erase(fmt_current_block);
   }

   if (ret == STATUS_SUCCESS)
   {
      ret = TABLE_Write(fmt_current_block, 0, &cached_area_table);
   }

   if (ret == STATUS_SUCCESS)
   {
      /* log the area table in index */
      index_table.area_index_table[area] = fmt_current_block;
      index_table.area_index_ect[area] = 0;
      /* log the area offset table */
      area_offset_table[area] = 0;

      fmt_current_block ++;
   }
   else
   {
      /* mark bad block, pick another block for area table. */
      ANCHOR_LogBadBlock(fmt_current_block);

      fmt_current_block = INVALID_BLOCK;
   }

   return fmt_current_block;
}


BOOL AREA_IsFull(PHY_BLOCK logical_block)
{
   STATUS   status;
   BOOL     ret = FALSE;
   AREA     area = AREA_INDEX(logical_block);

   status = AREA_Read(area);
   if (status == STATUS_SUCCESS)
   {
      /* offset is the current offset, so, -1 */
      if (area_offset_table[area] == PAGE_PER_PHY_BLOCK-1)
      {
         ret = TRUE;
      }
   }

   return ret;
}


BLOCK_OFF AREA_FindMinECBlock(AREA           area,
                              PHY_BLOCK*     physical_block,
                              ERASE_COUNT*   block_ec)
{
   STATUS      status;
   BLOCK_OFF   i;
   BLOCK_OFF   min_block_offset = 0;

   status = AREA_Read(area);
   if (status == STATUS_SUCCESS)
   {
      /* find the max ec block */
      for (i=0; i<CFG_PHY_BLOCK_PER_AREA; i++)
      {
         if (cached_area_table[i].physical_block_ec <
             cached_area_table[min_block_offset].physical_block_ec)
         {
            min_block_offset = i;
         }
      }

      *physical_block = cached_area_table[min_block_offset].physical_block;
      *block_ec = cached_area_table[min_block_offset].physical_block_ec;
   }

   return min_block_offset;
}


BOOL AREA_CheckUpdatePLR(PHY_BLOCK     logical_block,
                         PHY_BLOCK     origin_block,
                         ERASE_COUNT   block_ec)
{
   BOOL        need_plr = FALSE;
   BLOCK_OFF   block_offset = BLOCK_OFFSET_AREA(logical_block);

   /* the area has already been cached in ram table */
   if (cached_area_table[block_offset].physical_block == origin_block &&
       cached_area_table[block_offset].physical_block_ec == block_ec)
   {
      /* no need to update the area table */
      ;
   }
   else
   {
      /* the area table is not updated correctly for PL, continue updating */
      cached_area_table[block_offset].physical_block = origin_block;
      cached_area_table[block_offset].physical_block_ec = block_ec;

      need_plr = TRUE;
   }

   return need_plr;
}


STATUS AREA_Update(PHY_BLOCK      logical_block,
                   PHY_BLOCK      new_origin_block,
                   ERASE_COUNT    new_origin_ec)
{
   AREA        area;
   BLOCK_OFF   block;
   PAGE_OFF    page = INVALID_PAGE;
   STATUS      ret;

   block = BLOCK_OFFSET_AREA(logical_block);
   area = AREA_INDEX(logical_block);

   ret = AREA_Read(area);
   if (ret == STATUS_SUCCESS)
   {
      if (new_origin_block != INVALID_BLOCK)
      {
         cached_area_table[block].physical_block = new_origin_block;
         cached_area_table[block].physical_block_ec = new_origin_ec;
      }

      /* set the next area page offset */
      page = (area_offset_table[area]+1) % PAGE_PER_PHY_BLOCK;
      ret = TABLE_Write(index_table.area_index_table[area],
                        page,
                        &cached_area_table);
   }

   if (ret == STATUS_SUCCESS)
   {
      ASSERT(page != INVALID_PAGE);
      area_offset_table[area] = page;
   }
   else if (ret == STATUS_BADBLOCK)
   {
      /* bad block, set the offset to toggle reclaim of the area table block */
      area_offset_table[area] = PAGE_PER_PHY_BLOCK-1;
      index_table.area_index_ect[area] = INVALID_EC;
   }

   return ret;
}


PHY_BLOCK AREA_GetBlock(LOG_BLOCK logical_block)
{
   AREA        area;
   STATUS      status;
   PHY_BLOCK   block;
   PHY_BLOCK   ret = INVALID_BLOCK;

   block = BLOCK_OFFSET_AREA(logical_block);
   area = AREA_INDEX(logical_block);

   status = AREA_Read(area);
   if (status == STATUS_SUCCESS)
   {
      ret = cached_area_table[block].physical_block;
   }

   return ret;
}


ERASE_COUNT AREA_GetEC(PHY_BLOCK logical_block)
{
   AREA        area;
   STATUS      status;
   PHY_BLOCK   block;
   ERASE_COUNT ret = INVALID_EC;

   block = BLOCK_OFFSET_AREA(logical_block);
   area = AREA_INDEX(logical_block);

   status = AREA_Read(area);
   if (status == STATUS_SUCCESS)
   {
      ret = cached_area_table[block].physical_block_ec;
   }

   return ret;
}

void CalculateWLI(){
   ERASE_COUNT ec = INVALID_EC;
   STATUS      status;
   ERASE_COUNT E= 0;
   double sum=0;
   double temp= 1.0;
   int minus =0;
   for(int i = 0;i<4;i++){
       status = AREA_Read(i);
       for(int j = 0;j<512;j++){
         //  PRINTF("block EC = %d",cached_area_table[j].physical_block_ec);
          E+=cached_area_table[j].physical_block_ec;
       }

   }
   PRINTF("E: %d",E);
   for(int i = 0;i<4;i++){
       status = AREA_Read(i);
       for(int j = 0;j<512;j++){
          if(cached_area_table[j].physical_block_ec>=0 && cached_area_table[j].physical_block_ec!=INVALID_EC){
             minus= (double)cached_area_table[j].physical_block_ec*2048.0/(double)E-temp;
             if(minus<0){
                minus=minus*(-1);
             }
            //  PRINTF("MINUS:%d",(int)minus);
             sum+=minus;
          }
          
       }

   }
   sum=sum*1000/40.96;
   PRINTF("calculate over! WLI = %d /1000 percent \n",(int)sum);
   // float final = (float)sum;
   // PRINTF("calculate over! WLI = %f",final);
   
}


STATUS AREA_Read(AREA area)
{
   STATUS   ret = STATUS_SUCCESS;

   if (area != cached_area_number)
   {
      ASSERT(area != INVALID_AREA);
      ret = TABLE_Read(index_table.area_index_table[area],
                       &(area_offset_table[area]),
                       &(cached_area_table[0]));
   }

   if (ret == STATUS_SUCCESS)
   {
      cached_area_number = area;
   }
   else
   {
      cached_area_number = INVALID_AREA;
   }

   return ret;
}


