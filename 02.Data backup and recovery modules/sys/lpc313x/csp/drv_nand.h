/*************************************************************************
 *
 *   Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2008
 *
 *    File name   : drv_nand.h
 *    Description : lpc313x Nand driver heander file
 *
 *    History :
 *    1. Date        : 10.01.2010
 *       Author      : Stoyan Choynev
 *       Description : Initial Revision
 *
 *    $Revision: 30870 $
 **************************************************************************/
 
/** include files **/
#include <NXP\iolpc3130.h>
#include "arm_comm.h"

#ifndef __DRV_NAND_H
#define __DRV_NAND_H
/** definitions **/
#define FLASH_OK            ( 0 )
#define FLASH_ERROR         ( 1 )
// NAND BASE ADDRESS
#define FLASH_BASE_ADDR     0x70000000
// change 512 to 2048  (2021.1.21) 这个头文件似乎在系统中不起作用
#define NAND_MAIN_SIZE        (2048)     /*The NFC reads/writes 512 bytes of the main array*/
// change 16 to 64    (2021.1.21)
#define NAND_SPARE_SIZE       (64)      /*The NFC reads/writes 64 bytes of the spare array*/
#define NAND_PAGE_SIZE        (NAND_MAIN_SIZE+NAND_SPARE_SIZE)
#define NAND_BLK_SIZE         (131072)  /*128K(only main arrays)*/
#define NAND_PG_PER_BLK       (NAND_BLK_SIZE/NAND_MAIN_SIZE)  /**/
#define NAND_BLK_NUM          (4096)
#define NAND_PG_NUM           (NAND_BLK_NUM*NAND_PG_PER_BLK)

/** public data **/

/** public functions **/
void InitNAND(void);
/*************************************************************************
 * Function name: NandGetID
 * Parameters:  None
 *
 * Return: None
 *
 * Description: Reads Chip ID in to the RBA 0
 *
 *************************************************************************/
unsigned int NandGetID(void);
/*************************************************************************
 * Function Name: NandReadPage
 * Parameters: Page - NAND Flash Page Number. In is Block*NAND_PG_PER_BLK + Page_in_Block
 *             Buffer_Index - Index to NFC Buffer (0-3)
 * Return: FLASH_OK - No Read Error or 1bit Error
 *         FLASH_ERROR - Non Correcable Read Error
 * Description: Reads one page (512 main+16 spera) from NAND Flash into RBA
 *
 *************************************************************************/
unsigned int NandReadPage(unsigned char * dest, unsigned int Page,unsigned int Buff_Index);
/*************************************************************************
 * Function Name: NandEraseBlock
 * Parameters: Block - NAND Block Number.
 *
 * Return: FLASH_OK - Erase Successful
 *         FLASH_ERROR - Erase Fail
 * Description: Erase One Block
 *
 *************************************************************************/
Int32U NandEraseBlock(Int32U Block);

/*************************************************************************
 * Function Name: NandCheckBlock
 * Parameters: Block - NAND Block Number.
 *
 * Return: FLASH_OK -Valid Block
 *         FLASH_ERROR - Invalid Block
 * Description: Checks The Invalid Bytes in the Block
 *
 *************************************************************************/
unsigned int NandCheckBlock(unsigned char * dest, unsigned int Block);
/*************************************************************************
 * Function Name: NandWritePage
 * Parameters: Page - NAND Flash Page Number. In is Block*NAND_PG_PER_BLK + Page_in_Block
 *             Buffer_Index - Index to NFC Buffer (0-3)
 * Return: FLASH_OK - Write Successful
 *         FLASH_ERROR - Write Fail
 * Description: Writes one page (512 main+16 spera) from RBA
 *              into NAND Flash
 *************************************************************************/
unsigned int NandWritePage(unsigned char * src,unsigned int Page, unsigned int Buff_Index);
/*************************************************************************
 * Function Name: NandInvalidateBlock
 * Parameters: Block - NAND Block Number.
 *
 * Return: None
 *
 * Description: Writes Zero into Block Invalid Bytes
 *
 *************************************************************************/
void NandInvalidateBlock(unsigned int Block);
/*************************************************************************
 * Function Name: NandGetValidBlock
 * Parameters: Block - NAND Block Number. Start from this block
 *
 * Return: Block < NAND_BLK_NUM NAND -  Valid Block Number.
 *         NAND_BLK_NUM - No Valid Block
 * Description: Finds a vlaid Block.
 *
 *************************************************************************/
unsigned int NandGetValidBlock(unsigned int Block);

#endif /*__DRV_NAND_H*/
