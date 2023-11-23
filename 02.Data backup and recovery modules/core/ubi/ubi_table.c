/*********************************************************
 * Module name: ubi_table.c
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
 *    Basic function of table read/write.
 *
 *********************************************************/


#include <core\inc\cmn.h>
#include <core\inc\mtd.h>

#include <sys\sys.h>

#include "ubi_inc.h"


STATUS TABLE_Write(PHY_BLOCK block, PAGE_OFF page, void* buffer)
{
   STATUS   ret;
   SPARE    footprint;

   /* set footprint in spare data: mark 0 in first byte */
   footprint[0] = 0;

   /* write table, and footprint to mtd */
   ret = MTD_Program(block, page, buffer, footprint);
   if (ret == STATUS_SUCCESS)
   {
      ret = MTD_WaitReady(block);
   }

   return ret;
}


STATUS TABLE_Read(PHY_BLOCK block, PAGE_OFF* page, void* buffer)
{
   PAGE_OFF    head  = 0;
   PAGE_OFF    tail  = PAGE_PER_PHY_BLOCK;
   PAGE_OFF    mid;
   STATUS      ret;
   SPARE       footprint;

   ASSERT(page != NULL);
   if (*page != INVALID_PAGE)
   {
      tail = head = *page;
   }

   /* binary search in block, and read the valid table */
   mid = (head + tail) / 2;
   while (mid != head && mid != tail)
   {
      /* read in the mid page. Only read spare data as table's footprint */
      ret = MTD_Read(block, mid, NULL, footprint);
      if (ret == STATUS_SUCCESS)
      {
         if (footprint[0] == (UINT32)(-1))
         {
            /* ALL Fs, empty page, search upward */
            tail = mid;
         }
         else
         {
            /* valid page, search downward */
            head = mid;
         }
      }
      else
      {
         /* read failed, should be an ecc error, and may be cause by PL,
          * so, search upward.
          */
         tail = mid;
      }

      mid = (head + tail) / 2;
   }

   ASSERT(tail == head || tail == head+1);
   *page = mid;

   /* read out the page on head to QUEUE_TABLE */
   ret = MTD_Read(block, *page, buffer, footprint);
   if (ret == STATUS_SUCCESS)
   {
      if (footprint[0] == (UINT32)(-1))
      {
         /* all Fs, empty page */
         ret = STATUS_FAILURE;
      }
   }

   if (ret != STATUS_SUCCESS)
   {
      *page = INVALID_PAGE;
   }

   return ret;
}


