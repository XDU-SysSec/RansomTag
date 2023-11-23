/*********************************************************
 * Module name: mtd_nand.h
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
 *    NAND driver. HW controller dependent. 
 *
 *********************************************************/


#ifndef _MTD_NAND_H_
#define _MTD_NAND_H_


typedef UINT32       NAND_ROW;
typedef UINT16       NAND_COL;


/* all nand command bytes */
typedef enum {
   CMD_READ                   = 0x00,
   CMD_READ_COMMIT            = 0x30,
   CMD_READ_ID                = 0x90,
   CMD_RESET                  = 0xff,
   CMD_PAGE_PROGRAM           = 0x80,
   CMD_PAGE_PROGRAM_COMMIT    = 0x10,
   CMD_PROGRAM_FAKE_COMMIT    = 0x11,
   CMD_PAGE_FAKE_PROGRAM      = 0x81,
   CMD_BLOCK_ERASE            = 0x60,
   CMD_BLOCK_ERASE_COMMIT     = 0xd0,
   CMD_RANDOM_DATA_IN         = 0x85,
   CMD_RANDOM_DATA_OUT        = 0x05,
   CMD_RANDOM_DATA_OUT_COMMIT = 0xe0,
   CMD_READ_STATUS            = 0x70,
   CMD_READ_STATUS_DIE1       = 0xf1,
   CMD_READ_STATUS_DIE2       = 0xf2
} NAND_CMD;


/* status bit map */
#define NAND_STATUS_FAIL_BIT     (0x01)
#define NAND_STATUS_READY_BIT    (0x40)

#define NAND_STATUS_BUSY(s)      (((s)&NAND_STATUS_READY_BIT)==0)
#define NAND_STATUS_FAIL(s)      (((s)&NAND_STATUS_FAIL_BIT)==1)


/*********************************************************
 * Funcion Name: NAND_Init
 *
 * Description:
 *    Init the simulated nand.
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
void NAND_Init();


/*********************************************************
 * Funcion Name: NAND_SelectChip
 *
 * Description:
 *    Send chip select.
 *
 * Return Value:
 *    void
 *
 * Parameter List:
 *    chip     IN    selected chip number
 *
 * NOTES:
 *    N/A
 *
 *********************************************************/
void NAND_SelectChip(NAND_CHIP chip);


void NAND_DeSelectChip(NAND_CHIP chip);


/*********************************************************
 * Funcion Name: NAND_SendCMD
 *
 * Description:
 *    Send command data.
 *
 * Return Value:
 *    void
 *
 * Parameter List:
 *    cmd     IN    the command
 *
 * NOTES:
 *    N/A
 *
 *********************************************************/
void NAND_SendCMD(NAND_CMD cmd);


/*********************************************************
 * Funcion Name: NAND_SendAddr
 *
 * Description:
 *    Send column and row address in several cycles.
 *
 * Return Value:
 *    void
 *
 * Parameter List:
 *    col               IN    the column address
 *    row               IN    the row address
 *    col_cycle_count   IN    column address cycles
 *    row_cycle_count   IN    row address cycles
 *
 * NOTES:
 *    N/A
 *
 *********************************************************/
void NAND_SendAddr(NAND_COL col, NAND_ROW row, UINT8 col_cycle, UINT8 row_cycle);


/*********************************************************
 * Funcion Name: NAND_SendData
 *
 * Description:
 *    Send a sector of data to nand
 *
 * Return Value:
 *    void
 *
 * Parameter List:
 *    write_buffer   IN    the write_buffer holding the main data
 *    spare_data     IN    the 3-byte data in spare area
 *    spare_len      IN    the bytes of spare data
 *
 * NOTES:
 *    We can only write 3-byte data in spare area, all
 *    remaining are used for ECC code.
 *
 *********************************************************/
void NAND_SendData(unsigned char* write_buffer, SPARE spare_data);


/*********************************************************
 * Funcion Name: NAND_ReceiveData
 *
 * Description:
 *    Receive a sector of data from nand
 *
 * Return Value:
 *    STATUS      S/F
 *
 * Parameter List:
 *    write_buffer      IN    the write_buffer holding the main data
 *    spare_data  IN    receive the 3-byte spare data
 *    spare_len   IN    the bytes of spare data
 *
 * NOTES:
 *    return STATUS_FAILURE if ECC error detected after
 *    reading the sector from nand.
 *
 *********************************************************/
STATUS NAND_ReceiveData(unsigned char* write_buffer, SPARE spare_data);


/*********************************************************
 * Funcion Name: NAND_ReceiveBytes
 *
 * Description:
 *    Receive several bytes from nand, like ReadID and
 *    program status
 *
 * Return Value:
 *    void
 *
 * Parameter List:
 *    data_buffer    OUT      the buffers to hold byte data
 *    len            IN       the count of bytes to read
 *
 * NOTES:
 *    N/A
 *
 *********************************************************/
void NAND_ReceiveBytes(UINT8* data_buffer, UINT8 len);


/*********************************************************
 * Funcion Name: NAND_ECCStatus
 *
 * Description:
 *    Check ECC status after reading data from nand
 *
 * Return Value:
 *    BOOL     TRUE if ECC error detected.
 *
 * Parameter List:
 *    ecc_error_count   OUT      the ecc error bit count
 *
 * NOTES:
 *    return TRUE when ECC error detected!
 *
 *********************************************************/
BOOL NAND_ECCStatus(UINT8* ecc_error_count);

void NAND_WaitRB(NAND_CHIP chip_addr);

#endif


