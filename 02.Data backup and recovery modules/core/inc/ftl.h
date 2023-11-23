/*********************************************************
 * Module name: ftl_api.h
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
 *    FTL API.
 *
 *********************************************************/


#ifndef _INC_FTL_H_
#define _INC_FTL_H_


/*********************************************************
 * Funcion Name: FTL_Format
 *
 * Description:
 *    Create FTL image based on UBI.
 *
 * Return Value:
 *    STATUS      S/F
 *
 * Parameter List:
 *    N/A
 *
 * NOTES:
 *    Called before MTD_Init().
 *
 *********************************************************/
STATUS FTL_Format();


/*********************************************************
 * Funcion Name: FTL_Init
 *
 * Description:
 *    Init FTL. Read tables from nand, rebuild maptable,
 *    and do Power Loss Recovery if meeting PL.
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
STATUS FTL_Init();


/*********************************************************
 * Funcion Name: FTL_Write
 *
 * Description:
 *    Write a sector of data to a logical sector address.
 *
 * Return Value:
 *    STATUS      S/F
 *
 * Parameter List:
 *    addr     IN    the logical sector address
 *    buffer   IN    the data
 *
 * NOTES:
 *    Only one sector can be written by calling this API,
 *    but FTL will write_buffer sectors to write pages for better
 *    performance. The data of write_buffer is managed by FIFO,
 *    and pointed by a HANDLER, instead of a pointer.
 *
 *********************************************************/
STATUS FTL_Write(PGADDR addr, void* buffer,Boolean flag,Boolean mft);


/*********************************************************
 * Funcion Name: FTL_Read
 *
 * Description:
 *    Read a sector of data on a logical sector address.
 *
 * Return Value:
 *    STATUS      S/F
 *
 * Parameter List:
 *    addr     IN    the logical sector address
 *    buffer   OUT   the data
 *    flag     IN    false:read common data  true:read backup data
 *
 * NOTES:
 *    N/A
 *
 *********************************************************/
STATUS FTL_Read(PGADDR addr, void* buffer,Boolean flag , Boolean mft);


/*********************************************************
 * Funcion Name: FTL_Trim
 *
 * Description:
 *    Trim a continous region of sectors.
 *
 * Return Value:
 *    STATUS      S/F
 *
 * Parameter List:
 *    start    IN    the starting address of the sectors
 *    end      IN    the end address of the sectors
 *
 * NOTES:
 *    The sectors may not be aligned to the bondary of
 *    pages, nor blocks.
 *
 *********************************************************/
STATUS FTL_Trim(PGADDR start, PGADDR end);


/*********************************************************
 * Funcion Name: FTL_SetWP
 *
 * Description:
 *    Enable/Disable the write protection of a region of
 *    sectors.
 *
 * Return Value:
 *    STATUS      S/F
 *
 * Parameter List:
 *    laddr    IN    logical address of the starting sector
 *    enabled  IN    enable or disable write protection
 *
 * NOTES:
 *    For now, we can only set write protection by
 *    granuarity of PHY_BLOCK.
 *
 *********************************************************/
STATUS FTL_SetWP(PGADDR laddr, BOOL enabled);


/*********************************************************
 * Funcion Name: FTL_CheckWP
 *
 * Description:
 *    check the status of write protect of a region of
 *    sectors.
 *
 * Return Value:
 *    BOOL      enabled or disabled.
 *
 * Parameter List:
 *    laddr    IN    logical address of the sector in the
 *                   continous sector region.
 *
 * NOTES:
 *    For now, we can only set write protection by
 *    granuarity of MPP.
 *
 *********************************************************/
BOOL FTL_CheckWP(PGADDR laddr);


/*********************************************************
 * Funcion Name: FTL_Flush
 *
 * Description:
 *    Flush data in ram write_buffer to nand. Call when receives
 *    STOP cmd in SD bus.
 *
 * Return Value:
 *    STATUS      S/F
 *
 * Parameter List:
 *    N/A
 *
 * NOTES:
 *    Must be called after flushing write buffer.
 *
 *********************************************************/
STATUS FTL_Flush();


/*********************************************************
 * Funcion Name: FTL_BgTasks
 *
 * Description:
 *    The main entrance of the background task of FTL.
 *
 * Return Value:
 *    STATUS      S/F
 *
 * Parameter List:
 *    N/A
 *
 * NOTES:
 *    It should be called by system or bus/SD, when there
 *    is no active commands on the bus for a while.
 *
 *********************************************************/
STATUS FTL_BgTasks();


/*********************************************************
 * Funcion Name: FTL_Capacity
 *
 * Description:
 *    Return the capacity of the volumn managed by FTL.
 *
 * Return Value:
 *    PGADDR      the count of the user-aware sectors.
 *
 * Parameter List:
 *    N/A
 *
 * NOTES:
 *    USB need it. SD may not.
 *
 *********************************************************/
PGADDR FTL_Capacity();


#endif

