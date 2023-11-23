/*************************************************************************
 *
 *   Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2008
 *
 *    File name   : drv_nand.c
 *    Description : lpc313x Nand driver
 *
 *    History :
 *    1. Date        : 03.4.2009 ?
 *       Author      : Stoyan Choynev
 *       Description : Initial Revision
 *    2. Date        : 10.01.2010
 *
 *    $Revision: 30870 $
 **************************************************************************/

/** include files **/
#include "drv_cgu.h"
#include "drv_nand.h"

#include <core\inc\cmn.h>
#include <core\mtd\mtd_nand.h>


/** public functions **/
void NAND_Init()
{
   volatile Int32U i;

   /*Select EBI/MPMC pins*/
   SYSCREG_MUX_LCD_EBI_SEL_bit.Mux_LCD_EBI_sel = 1;

   CGU_Run_Clock(EBI_CLK);
   /*Enable MPMC clocks*/
   CGU_Run_Clock(MPMC_CFG_CLK);
   CGU_Run_Clock(MPMC_CFG_CLK2);
   CGU_Run_Clock(MPMC_CFG_CLK3);

   /* Enable NAND Flash Controller Clocks */
   CGU_Run_Clock(NANDFLASH_S0_CLK);
   CGU_Run_Clock(NANDFLASH_ECC_CLK);
   CGU_Run_Clock(NANDFLASH_NAND_CLK);
   CGU_Run_Clock(NANDFLASH_PCLK);

   /*Reset NAND Flash Controller*/
   NANDFLASH_CTRL_NAND_RESET_N_SOFT = 0;
   for(volatile int i = 0 ; 10000 > i; i++);
   NANDFLASH_CTRL_NAND_RESET_N_SOFT = 1;

   NANDFLASH_CTRL_ECC_RESET_N_SOFT = 0;
   for(volatile int i = 0 ; 10000 > i; i++);
   NANDFLASH_CTRL_ECC_RESET_N_SOFT = 1;

   /**/
   SYSCREG_MUX_NAND_MCI_SEL_bit.Mux_NAND_MCI_sel = 0;

   /* TODO: nand timing
    * - shorten cycle time: 1+1 
    * - use EDO when tRC<30ns
    * - enough power supply to NAND
    */
   NandTiming1 = 0x00030333;
   NandTiming2 = 0x03333333;

   /*Nand Configuration Register*/
   NandConfig = (1<<0)|     /* Error Correction On    */
                (0<<1)|     /* 8-bit flash device     */
                (0<<3)|     /* DMA disabled           */
                (0<<4)|     /* Little endian          */
                (0<<5)|     /* Latency configuration  */
                (0<<7)|     /* ECC started after 516B */
                (1<<8)|     /* CE deactivated         */
                (0<<10)|    /* 528 Bytes Read/Write   */
                (0<<12);    /* 5-bit mode ECC         */

   NandSetCE = 0x1E;
   /* NAND Reset */
   /*Clear Status Flags*/
   NandIRQStatusRaw = 0xffffffff;

   /*Erase Command*/
   NandSetCmd = CMD_RESET;
   // wait for device ready
   while (!NandIRQStatusRaw_bit.INT28R);
}


void NAND_SelectChip(NAND_CHIP chip)
{
   NandSetCE = 0xff & (((~(1<<chip)) | 1<<(chip+4)));
   //NandSetCE = 0x1f & (0x10 | (~(1<<chip)));
}

void NAND_DeSelectChip(NAND_CHIP chip)
{
   /* de-select all chip */
   NandSetCE = 0x0f;
}

void NAND_SendCMD(NAND_CMD cmd)
{
   /*Clear Status Flags*/
   NandIRQStatusRaw = 0xffffffff;

   NandSetCmd = cmd;
}

void NAND_SendAddr(NAND_COL col, NAND_ROW row, UINT8 col_cycle, UINT8 row_cycle)
{
   /*Column*/
   for(int i = 0 ; i<col_cycle ; i++)
   {
      /*Load Column*/
      NandSetAddr = col & 0xFF;
      /*Shift Column*/
      col>>=8;
   }

   /*Row*/
   for(int i = 0 ; i<row_cycle ; i++)
   {
      /*Load Row*/
      NandSetAddr = row & 0xFF;
      /*Shift Row*/
      row>>=8;
   }
}


/* TODO: in multi-channel architecture, the nand transfer time can further
 * be masked by MCU calculation and DMA across channels. In single channel
 * platform, we only use interleave to improve performance. Recommend 10 ch!
 */

void NAND_SendData(unsigned char* write_buffer, SPARE spare_data)
{
   unsigned int      spare_32;
   unsigned char*    dest;
   unsigned char     i;

   for (i=0; i<SECTOR_PER_PAGE; i++)
   {
      dest = (unsigned char *) 0x70000000;

      /*Clear Status flags*/
      NandIRQStatusRaw = 0xffffffff;
      /* send write buffer to dest1 */
      if (write_buffer != NULL)
      {
         memcpy(dest, write_buffer+i*SECTOR_SIZE, SECTOR_SIZE);
      }
      else
      {
         memset(dest, 0xff, SECTOR_SIZE);
      }

      /* send spare data for dest1 */
      if (spare_data != NULL && i < SPARE_WORDS_IN_PAGE)
      {
         spare_32 = spare_data[i];

         dest[SECTOR_SIZE]   = (UINT8)(spare_32>>24);
         dest[SECTOR_SIZE+1] = (UINT8)(spare_32>>16);
         dest[SECTOR_SIZE+2] = (UINT8)(spare_32>>8);
         dest[SECTOR_SIZE+3] = (UINT8)(spare_32);
      }
      else
      {
         dest[SECTOR_SIZE]   = 0;
         dest[SECTOR_SIZE+1] = 0;
         dest[SECTOR_SIZE+2] = 0;
         dest[SECTOR_SIZE+3] = 0;
      }

      /*Start data transfer*/
      /* TODO: can start transfer before ECC done? */
      //NandControlFlow = 0x10;
      NandControlFlow_bit.W0 = 1; //和上一句一个意思

      /*Wait ECC end*/
      while(!NandIRQStatusRaw_bit.INT20R);

      /*Wait trensfer end for dest1*/
      while(!NandIRQStatusRaw_bit.INT24R);
   }
}


STATUS NAND_ReceiveData(unsigned char* read_buffer, SPARE spare_data)
{
   unsigned int      spare_32;
   unsigned char*    src;
   unsigned char     i;

   for (i=0; i<SECTOR_PER_PAGE; i++)
   {
      src = (unsigned char *) 0x70000000;

      /* Clear Status flags */
      NandIRQStatusRaw = 0xffffffff;

      /* Start Reading to src1 */
      NandControlFlow = 1;

      /* wait reading done to src1 */
      while(!NandIRQStatusRaw_bit.INT22R);

      /* Wait reading and ECC to complete in src1 */
      while(!NandIRQStatusRaw_bit.INT21R);

      if (NandIRQStatusRaw_bit.INT26R || NandIRQStatusRaw_bit.INT27R)
      {
         /* return fail when reading erased (non_programmed) page */
         return STATUS_FAILURE;
      }

      /* TODO: return error if ECC happen, including correctable. use MASK-reg */
      if(NandIRQStatusRaw_bit.INT11R || NandIRQStatusRaw_bit.INT4R)
      {
         return STATUS_ECC_ERROR;
      }

      /* DMA data src1 */
      if (read_buffer != NULL)
      {
         memcpy(read_buffer+i*SECTOR_SIZE, src, SECTOR_SIZE);
      }

      if (spare_data != NULL && i < SPARE_WORDS_IN_PAGE)
      {
         spare_32 =               (src[SECTOR_SIZE]);
         spare_32 = spare_32<<8 | (src[SECTOR_SIZE+1]);
         spare_32 = spare_32<<8 | (src[SECTOR_SIZE+2]);
         spare_32 = spare_32<<8 | (src[SECTOR_SIZE+3]);

         spare_data[i] = spare_32;
      }

   }

   return STATUS_SUCCESS;
}


void NAND_ReceiveBytes(UINT8* data_buffer, UINT8 len)
{
   for(int i = 0; i < len; i++ )
   {
      for(volatile int (a) = 0; (a) < 100; (a)++);

      if (data_buffer != NULL)
      {
         data_buffer[i] = (UINT8)(NandReadData & 0x000000FF);
      }
   }
}

BOOL NAND_ECCStatus(UINT8* ecc_error_count)
{
   /* TODO: get value from register: NandECCErrStatus & NandIRQStatusRaw */
   *ecc_error_count = 8;

   return FALSE;
}

void NAND_WaitRB(NAND_CHIP chip_addr)
{
   if (chip_addr == 0)
   {
      while (!NandIRQStatusRaw_bit.INT28R);
   }
   else if (chip_addr == 1)
   {
      while (!NandIRQStatusRaw_bit.INT29R);
   }
   else if (chip_addr == 2)
   {
      while (!NandIRQStatusRaw_bit.INT30R);
   }
   else if (chip_addr == 3)
   {
      while (!NandIRQStatusRaw_bit.INT31R);
   }
   else
   {
      ASSERT(FALSE);
   }
}


