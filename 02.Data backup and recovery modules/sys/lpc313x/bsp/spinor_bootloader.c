/*************************************************************************
 *
 *    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2009
 *
 *    File name   : bootloader.c
 *    Description : lpc313x SPI bootloader - initialize SDRAM, copy image from
 *    SPI Flash memory to SDRAM memory and then jump to application entry point
 *
 *    History :
 *    1. Date        : April 4, 2009
 *       Author      : Stanimir Bonev
 *       Description : Initial revision
 *
 *    $Revision: 34106 $
 *
 **************************************************************************/
 
#include <NXP/iolpc3130.h>
#include "drv_cgu.h"
#include "drv_spi.h"
#include "drv_spinor.h"

/** local definitions **/

/*90MHz SDRAM Clock*/
#define SDRAM_BASE_ADDR       0x30000000
#define SDRAM_PERIOD          11.1 /*ns*/

#define P2C(Period)           (((Period<SDRAM_PERIOD)?0:(Int32U)((Flo32)Period/SDRAM_PERIOD))+1)

#define SDRAM_REFRESH         15625

#define SDRAM_TRP             20      /*ns*/
#define SDRAM_TRAS            48      /*ns*/
#define SDRAM_TAPR            2       /*ns*/
#define SDRAM_TWR             15      /*ns*/
#define SDRAM_TRC             72      /*ns*/
#define SDRAM_TRFC            80      /*ns*/
#define SDRAM_TXSR            80      /*ns*/
#define SDRAM_TDAL            5       /*Clocks*/
#define SDRAM_TRRD            2       /*Clocks*/
#define SDRAM_TMRD            2       /*Clocks*/

#pragma pack(1)

typedef union _Sec_level_boot_data_t
{
   unsigned char Data[5];
   struct
   {
      unsigned long DestAddr;
      unsigned char Type;
   };
} Sec_level_boot_data_t;

#pragma pack()

typedef enum _Sec_level_boot_page_type_t
{
   REGULAR_PAGE = 0, ENCRYPTED_PAGE,
   INVALID_PAGE = 0xFF
} Sec_level_boot_page_type_t;

#define SIZE_BOOTL2_DATA                sizeof(Sec_level_boot_data_t)
#define PAGES_PER_BLOCK                 8
#define BOOTLOADER_L2_IMAGE_MAX_SIZE   2*PAGES_PER_BLOCK  // 16 Pages (the actual size depends of page size)


void Dly_us(Int32U Dly)
{
   Dly *= 6;
   Timer0Ctrl = 0;               // stop counting
   Timer0Load =  Dly;            // load period
   Timer0Clear = 0;              // clear timer pending interrupt
   Timer0Ctrl_bit.Enable = 1;    // enable counting
   while(Timer0Value <= Dly);
   Timer0Ctrl_bit.Enable = 0;     // stop counting
}

void InitSDRAM(void)
{
   /*Select EBI/MPMC pins*/
   SYSCREG_MUX_LCD_EBI_SEL_bit.Mux_LCD_EBI_sel = 1;
   /*Enalbe EBI Clock*/
   CGU_Run_Clock(EBI_CLK);
   /*Enable MPMC clocks*/
   CGU_Run_Clock(MPMC_CFG_CLK);
   CGU_Run_Clock(MPMC_CFG_CLK2);
   CGU_Run_Clock(MPMC_CFG_CLK3);
   /*Enable TMR0 Clock. Used for SDRAM timing*/
   CGU_Run_Clock(TIMER0_PCLK);
   /*Enable MPMC */
   MPMCControl = 1;
   /*HCLK to MPMC_CLK ratio 1:1*/
   MPMCConfig_bit.CLK = 0;
   /*Set commad, address, and data delay */
   SYSCREG_MPMP_DELAYMODES_bit.MPMC_delaymodes0 = 0x24;
   /*Set data read delay*/
   SYSCREG_MPMP_DELAYMODES_bit.MPMC_delaymodes1 = 0x20;
   /**/
   SYSCREG_MPMP_DELAYMODES_bit.MPMC_delaymodes2 = 0x00;
   /*Command Delayed strategy*/
   MPMCDynamicReadConfig_bit.RD = 1;
   /*Memory Device type SDRAM*/
   MPMCDynamicConfig0_bit.MD = 0;
   /*512Mb (32Mx16), 4 banks, row length = 13, column length = 9*/
   MPMCDynamicConfig0_bit.AM = 0x0D;
   /*Buffer Disable*/
   MPMCDynamicConfig0_bit.B = 0x0;
   /*writes not protected*/
   MPMCDynamicConfig0_bit.P = 0x0;
   /*Configure RAS latency*/
   MPMCDynamicRasCas0_bit.RAS = 2;
   /*Configure CAS latency*/
   MPMCDynamicRasCas0_bit.CAS = 2;

   MPMCDynamictRP = P2C(SDRAM_TRP);
   MPMCDynamictRAS = P2C(SDRAM_TRAS);
   MPMCDynamictSREX = P2C(SDRAM_TXSR);
   MPMCDynamictAPR = SDRAM_TAPR;
   MPMCDynamictDAL = SDRAM_TDAL+P2C(SDRAM_TRP);
   MPMCDynamictWR = SDRAM_TWR;
   MPMCDynamictRC = P2C(SDRAM_TRC);
   MPMCDynamictRFC = P2C(SDRAM_TRFC);
   MPMCDynamictXSR = P2C(SDRAM_TXSR);
   MPMCDynamictRRD = SDRAM_TRRD;
   MPMCDynamictMRD = SDRAM_TMRD;
   /* JEDEC General SDRAM Initialization Sequence
    DELAY to allow power and clocks to stabilize ~100 us
    NOP*/
   Dly_us(100);

   MPMCDynamicControl = 0x4183;

   Dly_us(200);
   // PALL
   MPMCDynamicControl_bit.I = 2;

   MPMCDynamicRefresh = 1;

   Dly_us(250);

   MPMCDynamicRefresh = P2C(SDRAM_REFRESH) >> 4;
   // COMM
   MPMCDynamicControl_bit.I = 1;
   // Burst 8, Sequential, CAS-2
   volatile unsigned long Dummy = *(volatile unsigned int *)(SDRAM_BASE_ADDR | (0x23UL << 12));
   // NORM
   MPMCDynamicControl = 0x4000;
   MPMCDynamicConfig0_bit.B = 1;
}

/*************************************************************************
 * Function Name: bootloader
 * Parameters: none
 *
 * Return: none
 *
 * Description: initialize SDRAM, copy image from
 *    SPI Flash memory to SDRAM memory and then jump to application entry poin
 *
 *************************************************************************/
__arm void bootloader (void)
{

   union
   {
      unsigned char Data[4];
      unsigned int * pDest;
   } Addr;
   unsigned int * pSrc;
   unsigned int pageSize;
   unsigned int pageNum;
   unsigned int memoryPage;
   unsigned int RepPageSize;
   unsigned int TablePage;
   unsigned int TablePageOffset;
   unsigned char buffer[2112/sizeof(unsigned int)];
   unsigned char Table[2048/sizeof(unsigned int)];


   /*Init Clocks*/
   InitClock();
   /*SDRAM initialization*/
   InitSDRAM();
   /*SPI NOR initialization*/
   if(InitSPINOR())
   {
      while(1);
   }

   RepPageSize = pageSize = SPINOR_GetPageSize();
   pageNum =  SPINOR_GetPageNum();

   memoryPage = BOOTLOADER_L2_IMAGE_MAX_SIZE;
   RepPageSize &= ~0xFFUL;

   if (0 == (0xFF & pageSize))
   {
      // "binary" page size layout
      TablePageOffset  = 0;
      TablePage = memoryPage;
      memoryPage++;
      SPINOR_Read_Page(TablePage, Table);
   }

   SPINOR_Read_Page(memoryPage, (Int8U *)buffer);

   while(1)
   {
      Sec_level_boot_data_t * Boot_data;

      if(0 == (0xFF & pageSize))
      {
         Boot_data = (Sec_level_boot_data_t *)&Table[TablePageOffset];
         /*Check for valid data*/
         if(REGULAR_PAGE != Boot_data->Type) return;
         /*Copy Dest Address*/
         for(int i = 0; 4 > i; i++)
         {
            Addr.Data[i] = Boot_data->Data[i];
         }

         pSrc = (unsigned int *) buffer;
         /*Copy Data*/
         for(unsigned int i = 0 ; RepPageSize > i; i += sizeof(unsigned int))
         {
            *Addr.pDest++ = *pSrc++;
         }
         /**/
         TablePageOffset += SIZE_BOOTL2_DATA;
         /**/
         if ((pageSize - SIZE_BOOTL2_DATA) < TablePageOffset)
         {
            TablePageOffset  = 0;
            TablePage = ++memoryPage;
            /*Read new address table*/
            SPINOR_Read_Page(TablePage, Table);
         }
      }
      else
      {
         Boot_data = (Sec_level_boot_data_t *)&buffer[pageSize - 5];
         /*Check for valid data*/
         if(REGULAR_PAGE != Boot_data->Type) return;
         /*Copy Data*/
         for(int i = 0; 4 > i; i++)
         {
            Addr.Data[i] = Boot_data->Data[i];
         }

         //pSrc = (unsigned int *) (Boot_data->DestAddr);
         pSrc = (unsigned int *) buffer;

         for(unsigned int i = 0 ; RepPageSize > i; i += sizeof(unsigned int))
         {
            *Addr.pDest++ = *pSrc++;
         }
      }

      if(++memoryPage >= pageNum ) return;

      SPINOR_Read_Page(memoryPage, (Int8U *)buffer);
   }
}
