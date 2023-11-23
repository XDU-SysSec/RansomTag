/*********************************************************
 * Module name: mtd.h
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
 *    Memory Technology Device. Translate block and
 *    sector offset to physical chip/row/column address.
 *
 *********************************************************/


#ifndef _INC_MTD_H_
#define _INC_MTD_H_


/*********************************************************
 * Funcion Name: MTD_RawInit
 *
 * Description:
 *    Init MTD/nand without data in CFG table.
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
void MTD_Init();


/*********************************************************
 * Funcion Name: MTD_Reset
 *
 * Description:
 *    Reset the state of every nand device.
 *
 * Return Value:
 *    void
 *
 * Parameter List:
 *    N/A
 *
 * NOTES:
 *    N/A
 *
 *********************************************************/
void MTD_Reset();


/*********************************************************
 * Funcion Name: MTD_ReadID
 *
 * Description:
 *    Read ID of the nand chip.
 *
 * Return Value:
 *    STATUS      S/F
 *
 * Parameter List:
 *    chip     IN    the chip number
 *    id_data  OUT   the data read out
 *    bytes    IN    data cycle number
 *
 * NOTES:
 *    the bytes should be specified in CFG table
 *
 *********************************************************/
STATUS MTD_ReadID(NAND_CHIP chip, UINT8 id_data[], UINT8 bytes);


/*********************************************************
 * Funcion Name: MTD_Read
 *
 * Description:
 *    Read sectors from nand
 *
 * Return Value:
 *    STATUS      S/F
 *
 * Parameter List:
 *    block    IN    block number
 *    sector   IN    sector offset in the block
 *    count    IN    sector count to read
 *    queue    IN    get write_buffer from this queue to hold data
 *    flag     OUT   store the flag data in spare area
 *    pre_read IN    true if need to pre-read the next page
 *
 * NOTES:
 *    N/A
 *
 *********************************************************/
STATUS MTD_Read(PHY_BLOCK block, PAGE_OFF page, void* buffer, SPARE spare);


/*********************************************************
 * Funcion Name: MTD_Program
 *
 * Description:
 *    Write sectors to nand
 *
 * Return Value:
 *    STATUS      S/F/STATUS_BADBLOCK
 *
 * Parameter List:
 *    block    IN    block number
 *    page     IN    the page offset to program
 *    buffer   IN    the data
 *    spare    IN    the spare area
 *
 * NOTES:
 *    N/A
 *
 *********************************************************/
STATUS MTD_Program(PHY_BLOCK block, PAGE_OFF page, void* buffer, SPARE spare);


/*********************************************************
 * Funcion Name: MTD_Erase
 *
 * Description:
 *    Erase an block
 *
 * Return Value:
 *    STATUS      S/F/STATUS_BADBLOCK
 *
 * Parameter List:
 *    block    IN    block number
 *
 * NOTES:
 *    N/A
 *
 *********************************************************/
STATUS MTD_Erase(PHY_BLOCK block);


/*********************************************************
 * Funcion Name: MTD_CheckBlock
 *
 * Description:
 *    Check if the block is good or bad.
 *
 * Return Value:
 *    STATUS      S/F
 *
 * Parameter List:
 *    block    IN    block number
 *
 * NOTES:
 *    N/A
 *
 *********************************************************/
STATUS MTD_CheckBlock(PHY_BLOCK block);


/*********************************************************
 * Funcion Name: MTD_WaitReady
 *
 * Description:
 *    Wait on WIP program/erase.
 *
 * Return Value:
 *    STATUS      S/F
 *
 * Parameter List:
 *    block    IN    block number
 *
 * NOTES:
 *    N/A
 *
 *********************************************************/
STATUS MTD_WaitReady(PHY_BLOCK block);


/*********************************************************
 * Funcion Name: MTD_ReadStatus
 *
 * Description:
 *    Write sectors to nand
 *
 * Return Value:
 *    STATUS      S/F/STATUS_BADBLOCK
 *
 * Parameter List:
 *    block    IN    block number
 *    sector   IN    the starting sector offset in the
 *                   block
 *
 * NOTES:
 *    N/A
 *
 *********************************************************/
STATUS MTD_ReadStatus(PHY_BLOCK block);


#if (SIM_TEST == TRUE)
/*********************************************************
 * Funcion Name: MTD_TestBBR
 *
 * Description:
 *    BBR test engine: Set bad block failure.
 *
 * Return Value:
 *    void
 *
 * Parameter List:
 *    bbr_target     IN    target failure point
 *
 * NOTES:
 *    call before write or erase, then NAND_ReadStatus
 *    return failure
 *
 *********************************************************/
void MTD_TestBBR(UINT32 bbr_target);


/*********************************************************
 * Funcion Name: MTD_TestPLR
 *
 * Description:
 *    PLR test engine: Set power loss failure.
 *
 * Return Value:
 *    void
 *
 * Parameter List:
 *    plr_target     IN    target failure point
 *
 * NOTES:
 *    call before write and or, then NAND_CommitWrite/
 *    NAND_CommitErase return STATUS_SimulatedPowerLoss.
 *
 *********************************************************/
void MTD_TestPLR(UINT32 plr_target);


/*********************************************************
 * Funcion Name: MTD_TestECC
 *
 * Description:
 *    ECC test engine: Set ECC failure.
 *
 * Return Value:
 *    void
 *
 * Parameter List:
 *    N/A
 *
 * NOTES:
 *    call before read, then NAND_CheckECC return ecc error
 *
 *********************************************************/
void MTD_TestECC();


/*********************************************************
 * Funcion Name: MTD_TestReset
 *
 * Description:
 *    ECC test engine: reset.
 *
 * Return Value:
 *    void
 *
 * Parameter List:
 *    N/A
 *
 * NOTES:
 *    reset after testing
 *
 *********************************************************/
void MTD_TestReset();

#endif

#endif


