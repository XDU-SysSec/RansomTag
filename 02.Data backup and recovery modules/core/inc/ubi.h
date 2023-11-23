/*********************************************************
 * Module name: ubi.h
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

/**********************************************************
 * UBI_Init: init the context structure, read metadata from 
             MTD, and build the context in RAM. Repair if 
             data is dis-integrity due to power loss or other 
             problem. Rebuild the system if can not find or 
             not recover the metadata in MTD.
 **********************************************************
 * UBI_Read: (logical_block, page_offset), read one page 
             from MTD, converted to physical block. If ECC 
             failed, retry forever, and copy data to another 
             physical block, and forget the bad block.
 **********************************************************
 * UBI_Write:(logical_block, page_offset), write one page to 
             MTD. FTL trace the block space usage. If program 
             failed, copy data to another physical block, 
             and forget the bad block. The new selected 
             physical block should be the same die as the 
             bad block.
 **********************************************************
 * UBI_Erase:Change a physical block, chosen from the free 
             blocks, mapping to the logical block. The new 
             selected physical block should be the same die 
             as the bad block.
 **********************************************************
 * UBI_Flush:write all dirty data in RAM to MTD. Check the 
             interleave programming status, and handle 
             failure.
 **********************************************************/


#ifndef _UBI_H_
#define _UBI_H_


/* export UBI_Capacity */
#include <core\ubi\ubi_inc.h>
extern ANCHOR_TABLE anchor_table;
#define UBI_Capacity    (anchor_table.total_data_block)


/*********************************************************
 * Funcion Name: UBI_Format
 *
 * Description:
 *    Format UBI images.
 *
 * Return Value:
 *    STATUS      S/F
 *
 * Parameter List:
 *    N/A
 *
 * NOTES:
 *    N/A
 *
 *********************************************************/
STATUS UBI_Format();


/*********************************************************
 * Funcion Name: UBI_Init
 *
 * Description:
 *    Buld UBI running context with data in MTD.
 *
 * Return Value:
 *    STATUS      S/F
 *
 * Parameter List:
 *    N/A
 *
 * NOTES:
 *    N/A
 *
 *********************************************************/
STATUS UBI_Init();


/*********************************************************
 * Funcion Name: UBI_Read
 *
 * Description:
 *    Read data from UBI images.
 *
 * Return Value:
 *    STATUS      S/F
 *
 * Parameter List:
 *    block       IN       logical block number
 *    page        IN       page in the block
 *    buffer      OUT      data buffer
 *    spare       OUT      spare data buffer
 *
 * NOTES:
 *    N/A
 *
 *********************************************************/
STATUS UBI_Read(LOG_BLOCK block, PAGE_OFF page, void* buffer, SPARE spare);


/*********************************************************
 * Funcion Name: UBI_Write
 *
 * Description:
 *    Write data to UBI images.
 *
 * Return Value:
 *    STATUS      S/F
 *
 * Parameter List:
 *    block       IN       logical block number
 *    page        IN       page in the block
 *    buffer      IN       data buffer
 *    spare       IN       spare data buffer
 *    async       IN       interleave write flag
 *
 * NOTES:
 *    N/A
 *
 *********************************************************/
STATUS UBI_Write(LOG_BLOCK block, PAGE_OFF page, void* buffer, SPARE spare, BOOL async);


/*********************************************************
 * Funcion Name: UBI_Erase
 *
 * Description:
 *    Erase a logical block.
 *
 * Return Value:
 *    STATUS      S/F
 *
 * Parameter List:
 *    block       IN       logical block number
 *    die_index   IN       the die where the block is.
 *
 * NOTES:
 *    Choose another physical block in the same die for 
 *    the logical block.
 *
 *********************************************************/
STATUS UBI_Erase(LOG_BLOCK block, LOG_BLOCK die_index);


/*********************************************************
 * Funcion Name: UBI_Flush
 *
 * Description:
 *    Flush data cache, and check the program status.
 *
 * Return Value:
 *    STATUS      S/F
 *
 * Parameter List:
 *    N/A
 *
 * NOTES:
 *    N/A
 *
 *********************************************************/
STATUS UBI_Flush();


/*********************************************************
 * Funcion Name: UBI_SWL
 *
 * Description:
 *    Static Wear Leveling.
 *
 * Return Value:
 *    STATUS      S/F
 *
 * Parameter List:
 *    N/A
 *
 * NOTES:
 *    Run this function in background.
 *
 *********************************************************/
STATUS UBI_SWL();


/*********************************************************
 * Funcion Name: UBI_ReadStatus
 *
 * Description:
 *    Get the status of the physical block.
 *
 * Return Value:
 *    STATUS      S/F
 *
 * Parameter List:
 *    block       IN    the block number
 *
 * NOTES:
 *    N/A
 *
 *********************************************************/
STATUS UBI_ReadStatus(LOG_BLOCK block);

#endif


