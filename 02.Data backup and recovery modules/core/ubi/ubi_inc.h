/*********************************************************
 * Module name: ubi_inc.h
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
 *    UBI data structures.
 *
 *********************************************************/


#ifndef _UBI_TABLE_H_
#define _UBI_TABLE_H_


/* define block index table */
#define FREE_BLOCK_COUNT            \
      ((MPP_SIZE/4-sizeof(AREA_UPDATE_PLR))/sizeof(PHY_BLOCK))
/** 预留用作坏块替换的block个数*/
#define REAL_FREE_BLOCK_COUNT       \
      (CFG_LOG_BLOCK_COUNT/(100/(100-GOOD_BLOCK_PERCENT)))
/**意义不明*/
#define ANCHOR_BLOCK_COUNT          \
      (CFG_LOG_BLOCK_COUNT*2*2/PAGE_PER_PHY_BLOCK/PAGE_PER_PHY_BLOCK + 2) 
// 意义不明，-11是什么意思
#define ANCHOR_BADBLOCK_COUNT       (MPP_SIZE/sizeof(PHY_BLOCK)-11)

#define ANCHOR_OATH                 ("I LOVE WSS!")
#define ANCHOR_VERSION              (123)


/* block 0 is reserved for code, MCU boot will read and copy it. 
 * Block 0 contains 2 blocks in 2 plane. 
 * the first block has NAND parameter table
 * the second block has the code image with the valid header.
 * If any block is bad, fail to use the flash!!! 
 */
#define CODE_BLOCK                  (0)
#define CODE_BLOCK_COUNT            (1)

#define ANCHOR_FIRST_BLOCK          (CODE_BLOCK+CODE_BLOCK_COUNT)  // 0+1 =1
#define ANCHOR_LAST_BLOCK           (ANCHOR_FIRST_BLOCK+ANCHOR_BLOCK_COUNT-1)

#define INDEX_BLOCK_COUNT           (1)

#define DATA_FIRST_BLOCK            (ANCHOR_FIRST_BLOCK+ANCHOR_BLOCK_COUNT)
/**
 * 数据部分占用的block= 逻辑块总数 - CODE_Block - ANCHOR_BLOCK - INDEX_BLOCK - REAL_FREE_BLOCK
 */
#define DATA_BLOCK_COUNT            (CFG_LOG_BLOCK_COUNT -  \
                                     CODE_BLOCK_COUNT -     \
                                     ANCHOR_BLOCK_COUNT -   \
                                     INDEX_BLOCK_COUNT -    \
                                     REAL_FREE_BLOCK_COUNT)
// 这个block所在的AREA的索引
#define AREA_INDEX(block)           ((AREA)((block) / CFG_PHY_BLOCK_PER_AREA))
// block在AREA内部的offset
#define BLOCK_OFFSET_AREA(block)    ((BLOCK_OFF)((block)%CFG_PHY_BLOCK_PER_AREA))


typedef struct {
   PHY_BLOCK      area_updating_logical_block;
   PHY_BLOCK      area_updating_physical_block;
   ERASE_COUNT    area_updating_block_ec;
} AREA_UPDATE_PLR;

typedef struct {
   PHY_BLOCK         area_index_table[AREA_TABLE_SIZE];
   ERASE_COUNT       area_index_ect[AREA_TABLE_SIZE];
   PHY_BLOCK         free_block_table[FREE_BLOCK_COUNT];
   ERASE_COUNT       free_block_ect[FREE_BLOCK_COUNT];
   AREA_UPDATE_PLR   area_update_plr;
   AREA_UPDATE_PLR   padding;
} INDEX_TABLE;

typedef struct
{
   /* INDEX block plr */
   PHY_BLOCK      index_new_block;
   ERASE_COUNT    index_new_ec;
   PHY_BLOCK      index_old_block;
   ERASE_COUNT    index_old_ec;

   /* anchor block plr */
   PHY_BLOCK      previous_anchor_block;

   /* total data block */
   PHY_BLOCK      total_data_block;

   /* swl block tracker */
   AREA           swl_current_area;

   /* identity of correct image with code */
   UINT8          oath[12];
   UINT32         version;

   /* bad block table */
   PHY_BLOCK      bad_block_table[ANCHOR_BADBLOCK_COUNT];
} ANCHOR_TABLE;


extern INDEX_TABLE index_table;

extern ANCHOR_TABLE anchor_table;


/***************************************************
 * Funcion Name: INDEX_Init
 *
 * Description:
 *    Init the index sub-module.
 *
 * Return Value:
 *    STATUS      S/F
 *
 * Parameter List:
 *    logical_block  OUT   updating logical block in
 *                         AREA updating PLR info
 *    origin_block   OUT   origin block number of the
 *                         updating logical block in
 *                         AREA updating PLR info
 *    block_ec       OUT   erase count of the origin
 *                         block.
 *
 * NOTES:
 *    In index table, we store the updating AREA table
 *    info to avoid dis-integrity issue after an PL.
 *    We can updating AREA table in PLR progress to
 *    a integrity state with index table.
 *    When get the plr info, read earilier page as valid
 *    index table, and read earilier page in area block
 *    as valid area index table. set offset properly
 *    (write something in obsoloted page in area block).
 *
 ***************************************************/
STATUS INDEX_Init(PHY_BLOCK*        logical_block,
                  PHY_BLOCK*        origin_block,
                  ERASE_COUNT*      block_ec);


/***************************************************
 * Funcion Name: INDEX_Format
 *
 * Description:
 *    Write the index table to index table block
 *    for the first time, as formatting the nand.
 *
 * Return Value:
 *    PHY_BLOCK       index table block number
 *
 * Parameter List:
 *    total_block    IN    the total block count in
 *                         all nand chip
 *    fmt_current_block    the current block that is
 *                   IN    not formatted.
 *
 * NOTES:
 *    N/A
 *
 ***************************************************/
PHY_BLOCK INDEX_Format(PHY_BLOCK   total_block,
                       PHY_BLOCK   fmt_current_block);


/***************************************************
 * Funcion Name: INDEX_Update_AreaReclaim
 *
 * Description:
 *    update the index table in xram for area table
 *    block reclaim
 *
 * Return Value:
 *    void
 *
 * Parameter List:
 *    area        IN    the area block reclaimed
 *    new_block   IN    new area table block
 *    new_ec      IN    erase count of new area block
 *
 * NOTES:
 *    Update index table in xram first, and sync to
 *    nand to keep nand data state integrity.
 *
 ***************************************************/
void INDEX_Update_AreaReclaim(AREA area, PHY_BLOCK new_block, ERASE_COUNT nec);


/***************************************************
 * Funcion Name: INDEX_Update_AreaUpdate
 *
 * Description:
 *    update the index table in xram for updating area
 *    table without reclaim.
 *
 * Return Value:
 *    void
 *
 * Parameter List:
 *    logical_block     IN    the updating logical block
 *    physical_block    IN    updating physical block as
 *                            new origin block of the
 *                            logical block.
 *    block_ec          IN    erase count of the updating
 *                            physical block
 *
 * NOTES:
 *    Update index table in xram first, and sync to
 *    nand to keep nand data state integrity.
 *
 ***************************************************/
void INDEX_Update_AreaUpdate(PHY_BLOCK     logical_block,
                             PHY_BLOCK     physical_block,
                             ERASE_COUNT   block_ec);


/***************************************************
 * Funcion Name: INDEX_Update_Commit
 *
 * Description:
 *    Commit/Sync index table in xram to nand.
 *
 * Return Value:
 *    void
 *
 * Parameter List:
 *    N/A
 *
 * NOTES:
 *    Update index table in xram first, and sync to
 *    nand to keep nand data state integrity. Update
 *    PLR info of index table block to CFG table when
 *    reclaiming index table block.
 *
 ***************************************************/
STATUS INDEX_Update_Commit();


/***************************************************
 * Funcion Name: INDEX_FreeBlock_SwapMaxECBlock
 *
 * Description:
 *    In SWL, swap a least erased origin block
 *    with a most erased block in free table.
 *
 * Return Value:
 *    void
 *
 * Parameter List:
 *    physical_block    IN    the physical block number
 *                            of the least erased.
 *    block_ec          IN    the erase count of the block
 *
 * NOTES:
 *    Update index table in xram first, and sync to
 *    nand to keep nand data state integrity.
 *
 ***************************************************/
void INDEX_FreeBlock_SwapMaxECBlock(PHY_BLOCK      min_ec_block,
                                    ERASE_COUNT    min_ec);


/***************************************************
 * Funcion Name: INDEX_FreeBlock_GetMaxECBlock
 *
 * Description:
 *    Find the free block with max EC. 
 *
 * Return Value:
 *    void
 *
 * Parameter List:
 *    physical_block    OUT      the block number
 *    block_ec          OUT      the max ec
 *
 * NOTES:
 *    Update index table in xram first, and sync to
 *    nand to keep nand data state integrity.
 *
 ***************************************************/
void INDEX_FreeBlock_GetMaxECBlock(PHY_BLOCK*      physical_block,
                                   ERASE_COUNT*    block_ec);


/***************************************************
 * Funcion Name: INDEX_FreeBlock_Put
 *
 * Description:
 *    put reclaim block to free table, which can be
 *    used later.
 *
 * Return Value:
 *    void
 *
 * Parameter List:
 *    dirty_block    IN    the physical block number
 *                         of the reclaimed dirty block.
 *    dirty_block_ec IN    the erase count of the block
 *
 * NOTES:
 *    Only erase the block bofore getting it from
 *    the free table.
 *
 ***************************************************/
void INDEX_FreeBlock_Put(PHY_BLOCK dirty_block, ERASE_COUNT dirty_block_ec);


/***************************************************
 * Funcion Name: INDEX_FreeBlock_Get
 *
 * Description:
 *    get a least erased block from the free table.
 *
 * Return Value:
 *    STATUS      S/F/STATUS_TOOMANY_BADBLOCK
 *
 * Parameter List:
 *    block    OUT      the least erased block number
 *    ec       OUT      the erase count of the block
 *
 * NOTES:
 *    Only erase the block bofore getting it from
 *    the free table.
 *
 ***************************************************/
STATUS INDEX_FreeBlock_Get(DIE_INDEX die, PHY_BLOCK* block, ERASE_COUNT* ec);


/***************************************************
 * Funcion Name: AREA_Init
 *
 * Description:
 *    Init the area sub-module.
 *
 * Return Value:
 *    STATUS      S/F/STATUS_BADBLOCK
 *
 * Parameter List:
 *    area_index    IN      the area index to init
 *
 * NOTES:
 *    Just init the one area. Caller repeats calling.
 *
 ***************************************************/
STATUS AREA_Init(AREA area_index);


/***************************************************
 * Funcion Name: AREA_Format
 *
 * Description:
 *    Write area table to area table block for the
 *    first time, as the progress of formatting.
 *
 * Return Value:
 *    STATUS      S/F/STATUS_BADBLOCK
 *
 * Parameter List:
 *    block_count    IN    block count in this area
 *    fmt_current_block    current block that is
 *                   IN    not formatted
 *    area           IN    the area index to write
 *
 * NOTES:
 *    Just format the one area. Caller repeats calling.
 *
 ***************************************************/
PHY_BLOCK AREA_Format(PHY_BLOCK     block_count,
                      PHY_BLOCK     fmt_current_block,
                      AREA          area);


/***************************************************
 * Funcion Name: AREA_Update
 *
 * Description:
 *    update area table for replace a origin block
 *
 * Return Value:
 *    STATUS      S/F
 *
 * Parameter List:
 *    logical_block     IN    the logical block number
 *                            to replace.
 *    new_origin_block  IN    new origin block for the
 *                            logical block
 *    new_origin_ec     IN    the erase count of the block
 *
 * NOTES:
 *    N/A
 *
 ***************************************************/
STATUS AREA_Update(PHY_BLOCK        logical_block,
                   PHY_BLOCK        new_origin_block,
                   ERASE_COUNT      new_origin_ec);


/***************************************************
 * Funcion Name: AREA_IsFull
 *
 * Description:
 *    check if the area table block is full
 *
 * Return Value:
 *    BOOL        TRUE if full
 *
 * Parameter List:
 *    logical_block     IN    the logical block number
 *                            belonging to the area
 *
 * NOTES:
 *    N/A
 *
 ***************************************************/
BOOL AREA_IsFull(PHY_BLOCK logical_block);


/***************************************************
 * Funcion Name: AREA_FindMinECBlock
 *
 * Description:
 *    In SWL, swap a least erased origin block
 *    with a most erased block in free table.
 *
 * Return Value:
 *    BLOCK_OFF        block offset in the area
 *
 * Parameter List:
 *    area              IN    find the least erased
 *                            block in this area
 *    physical_block    OUT   the least erased
 *                            origin block in the area
 *    block_ec          OUT   erase count of the block
 *
 * NOTES:
 *    N/A
 *
 ***************************************************/
BLOCK_OFF AREA_FindMinECBlock(AREA           area,
                              PHY_BLOCK*     physical_block,
                              ERASE_COUNT*   block_ec);


/***************************************************
 * Funcion Name: AREA_CheckUpdatePLR
 *
 * Description:
 *    in init/PLR, check the area table if integrity
 *    with index table.
 *
 * Return Value:
 *    BOOL     TRUE if NOT integrity
 *
 * Parameter List:
 *    logical_block     IN    the updating logical block
 *    origin_block      IN    updating physical block as
 *                            new origin block of the
 *                            logical block.
 *    block_ec          IN    erase count of the updating
 *                            physical bloc
 *
 * NOTES:
 *    If area table is NOT integrity with index table,
 *    the area table should be written again during
 *    init with updated origin block number and ec.
 *
 ***************************************************/
BOOL AREA_CheckUpdatePLR(PHY_BLOCK     logical_block,
                         PHY_BLOCK     origin_block,
                         ERASE_COUNT   block_ec);


/***************************************************
 * Funcion Name: AREA_Read
 *
 * Description:
 *    read the area table from area table block to
 *    xram.
 *
 * Return Value:
 *    STATUS         S/F
 *
 * Parameter List:
 *    area        IN    area table to read
 *
 * NOTES:
 *    Also need to update the area offset table.
 *
 ***************************************************/
STATUS AREA_Read(AREA area);


void CalculateWLI();


/***************************************************
 * Funcion Name: AREA_GetBlock
 *
 * Description:
 *    get physical origin block of the logical block
 *
 * Return Value:
 *    PHY_BLOCK       the physical origin block
 *
 * Parameter List:
 *    logical_block     IN    the logical block number
 *                            to inquire.
 *
 * NOTES:
 *    N/A
 *
 ***************************************************/
PHY_BLOCK AREA_GetBlock(PHY_BLOCK logical_block);


/***************************************************
 * Funcion Name: AREA_GetEC
 *
 * Description:
 *    get erase count of the physical origin block
 *    of the logical block
 *
 * Return Value:
 *    ERASE_COUNT       the erase count of the
 *                      physical origin block
 *
 * Parameter List:
 *    logical_block     IN    the logical block number
 *                            to inquire.
 *
 * NOTES:
 *    N/A
 *
 ***************************************************/
ERASE_COUNT AREA_GetEC(PHY_BLOCK logical_block);


/***************************************************
 * Funcion Name: TABLE_Write
 *
 * Description:
 *    Write table (e.g. index table, area table,
 *    cfg table) to a block.
 *
 * Return Value:
 *    STATUS         S/F
 *
 * Parameter List:
 *    block       IN    the physical block to write
 *    page        IN    the page offset to write
 *    sector_count      the sector count of the table
 *                IN    to write to nand.
 *
 * NOTES:
 *    Write table with footprint info in spare area.
 *    We scanthe footprint to find the latest updated
 *    page. And, the table's size must be NOT greater than
 *    a page.
 *
 ***************************************************/
STATUS TABLE_Write(PHY_BLOCK block, PAGE_OFF page, void* buffer);


/***************************************************
 * Funcion Name: TABLE_Read
 *
 * Description:
 *    read table (e.g. index table, area table,
 *    cfg table) from a block.
 *
 * Return Value:
 *    STATUS         S/F
 *
 * Parameter List:
 *    block       IN    the physical block to read
 *    page        OUT   the page offset of the valid
 *                      and up-to-date table
 *    sector_count      the sector count of the table
 *                IN    to read
 *    queue       IN    get buffers holding data from
 *                      this queue.
 *
 * NOTES:
 *    Write table with footprint info in spare area.
 *    We scanthe footprint to find the latest updated
 *    page. And, the table's size must be NOT greater than
 *    a page.
 *
 *    For parameter page:
 *    IN:  INVALID_PAGE, means search and read the table
 *    OUT: INVALID_PAGE, when find empty page
 *
 ***************************************************/
STATUS TABLE_Read(PHY_BLOCK block, PAGE_OFF* page, void* buffer);


/***************************************************
 * Funcion Name: ANCHOR_Format
 *
 * Description:
 *    Format anchor blocks
 *
 * Return Value:
 *    STATUS         S/F
 *
 * Parameter List:
 *    total_data_block  IN    the data block count 
 *
 * NOTES:
 *    N/A
 *
 ***************************************************/
STATUS ANCHOR_Format(PHY_BLOCK total_data_block);


/***************************************************
 * Funcion Name: ANCHOR_Init
 *
 * Description:
 *    Init anchor with data in blocks.
 *
 * Return Value:
 *    STATUS         S/F
 *
 * Parameter List:
 *    N/A
 *
 * NOTES:
 *    N/A
 *
 ***************************************************/
STATUS ANCHOR_Init();


/***************************************************
 * Funcion Name: ANCHOR_Update
 *
 * Description:
 *    Update the latest anchor table to blocks.
 *
 * Return Value:
 *    STATUS         S/F
 *
 * Parameter List:
 *    N/A
 *
 * NOTES:
 *    N/A
 *
 ***************************************************/
STATUS ANCHOR_Update();


/***************************************************
 * Funcion Name: ANCHOR_LogBadBlock
 *
 * Description:
 *    Log the bad block number to anchor table.
 *
 * Return Value:
 *    N/A
 *
 * Parameter List:
 *    block       IN    the block number
 *
 * NOTES:
 *    N/A
 *
 ***************************************************/
void ANCHOR_LogBadBlock(PHY_BLOCK block);


/***************************************************
 * Funcion Name: ANCHOR_IsBadBlock
 *
 * Description:
 *    Check in bad block table if the block is bad.
 *
 * Return Value:
 *    BOOL
 *
 * Parameter List:
 *    block       IN    the block number
 *
 * NOTES:
 *    N/A
 *
 ***************************************************/
BOOL ANCHOR_IsBadBlock(PHY_BLOCK block);


/***************************************************
 * Funcion Name: ANCHOR_IndexReclaim
 *
 * Description:
 *    Log the index reclaim info in anchor table, for
 *    solving PLR issue.
 *
 * Return Value:
 *    BOOL
 *
 * Parameter List:
 *    block       IN    the block number
 *
 * NOTES:
 *    N/A
 *
 ***************************************************/
void ANCHOR_IndexReclaim(PHY_BLOCK     index_current_block,
                         ERASE_COUNT   index_current_ec,
                         PHY_BLOCK     new_itb,
                         ERASE_COUNT   new_itb_ec);

#endif


