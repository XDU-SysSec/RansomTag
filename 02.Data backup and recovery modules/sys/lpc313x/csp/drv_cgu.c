/*************************************************************************
 *
 *   Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2009
 *
 *    File name   : drv_cgu.c
 *    Description : Lpc3130 Clock Generation Unit Driver
 *
 *    History :
 *    1. Date        : 30.3.2009
 *       Author      : Stoyan Choynev
 *       Description : Initila Revision
 *
 *    $Revision: 31493 $
 *************************************************************************/
/** include files **/
#include <NXP\iolpc3130.h>
#include "arm_comm.h"
#include "drv_cgu.h"

/** local definitions **/
#define FDIV_MADD_MSUB(n,m)   ((((m-n)&0xFF)<<3) | (((-n)&0xFF)<<11))
#define FDIV17_MADD_MSUB(n,m) ((((m-n)&0x1FFF)<<3) | (((-n)&0x1FFF)<<16))
#define FDCTRL_STRETCH (1<<2)
#define FDCTRL_ENABLE  (1<<0)

#define ESR_ENABLE (1<<0)
#define ESR_FDIV(n) ((n<<1)|ESR_ENABLE)
#define BASE_CLOCK 0


/** default settings **/
#define CLK_EN_0_31   ((1<<APB0_CLK)|(1<<APB1_CLK)|(1<<APB2_CLK)|(1<<APB3_CLK)|(1<<APB4_CLK)|\
                       (1<<AHB_TO_INTC_CLK)|(1<<AHB0_CLK)|(1<<CLOCK_OUT)|\
                       (1<<ARM926_CORE_CLK)|(1<<ARM926_BUSIF_CLK)|(1<<ARM926_RETIME_CLK)|(1<<ISRAM0_CLK)|\
                       (1<<ISRAM1_CLK)|(1<<ISROM_CLK)|(1<<INTC_CLK)|(1<<AHB_TO_APB0_PCLK))
#define CLK_EN_32_63  ((1<<(IOCONF_PCLK-32))|(1<<(CGU_PCLK-32))|(1<<(SYSCREG_PCLK-32))|(1<<(AHB_TO_APB1_PCLK-32))|(1<<(AHB_TO_APB2_PCLK-32))|\
                       (1<<(PCM_PCLK-32))|(1<<(AHB_TO_APB3_PCLK-32)))
#define CLK_EN_64_92  (0)

/** external functions **/

/** external data **/

/** internal functions **/
void CGU_SelectDomainClk(DOMAIN Domain, IN_CLOCKS Clock);
void CGU_CnfgBCR(DOMAIN Domain,Int32U Cnfg);

/** public data **/

/** private data **/
static const Int32U FDIV_Config[FDIV23+1]  =
{
  /*SYS*/
  FDIV_MADD_MSUB(1,2)|FDCTRL_STRETCH|FDCTRL_ENABLE,  /*FDIV0 1/2*/
  FDIV_MADD_MSUB(1,2)|FDCTRL_STRETCH,                /*FDIV1 1/2*/
  FDIV_MADD_MSUB(1,4)|FDCTRL_STRETCH|FDCTRL_ENABLE,  /*FDIV2 1/4*/
  FDIV_MADD_MSUB(1,4)|FDCTRL_STRETCH,                /*FDIV3 1/4*/
  FDIV_MADD_MSUB(1,3)|FDCTRL_STRETCH|FDCTRL_ENABLE,  /*FDIV4 1/3*/
  FDIV_MADD_MSUB(1,8)|FDCTRL_STRETCH|FDCTRL_ENABLE,  /*FDIV5 1/8*/
  FDIV_MADD_MSUB(1,8)|FDCTRL_STRETCH,                /*FDIV6 1/8*/
  /*AHB_APB0_BASE*/
  FDIV_MADD_MSUB(1,4)|FDCTRL_STRETCH|FDCTRL_ENABLE,  /*FDIV7 1/2*/
  FDIV_MADD_MSUB(1,180)|FDCTRL_STRETCH|FDCTRL_ENABLE,/*FDIV8 1/180*/
  /*AHB_APB1_BASE*/
  FDIV_MADD_MSUB(1,2)|FDCTRL_STRETCH|FDCTRL_ENABLE,  /*FDIV9 1/2*/
  FDIV_MADD_MSUB(1,12)|FDCTRL_STRETCH|FDCTRL_ENABLE, /*FDIV10 1/12*/
  /*AHB_APB2_BASE*/
  FDIV_MADD_MSUB(1,2)|FDCTRL_STRETCH|FDCTRL_ENABLE,  /*FDIV11 1/2*/
  FDIV_MADD_MSUB(1,40)|FDCTRL_STRETCH,               /*FDIV12 1/40*/
  FDIV_MADD_MSUB(1,16)|FDCTRL_STRETCH,               /*FDIV13 1/16*/
  /*AHB_APB3_BASE*/
  FDIV_MADD_MSUB(1,2)|FDCTRL_STRETCH,                /*FDIV14 1/2*/
  /*PCM_BASE */
  FDIV_MADD_MSUB(1,8)|FDCTRL_STRETCH|FDCTRL_ENABLE,  /*FDIV15 1/8*/
  /*UART_BASE*/
  FDIV_MADD_MSUB(1,2)|FDCTRL_STRETCH,                /*FDIV16 1/2*/
  /*CLK1024FS_BASE*/
  FDIV17_MADD_MSUB(1,256)|FDCTRL_STRETCH|FDCTRL_ENABLE, /*FDIV17 1/256*/
  FDIV_MADD_MSUB(1,4)|FDCTRL_STRETCH|FDCTRL_ENABLE,     /*FDIV18 1/4  */
  FDIV_MADD_MSUB(1,2)|FDCTRL_STRETCH,                   /*FDIV19 1/2*/
  FDIV_MADD_MSUB(1,4)|FDCTRL_STRETCH|FDCTRL_ENABLE,     /*FDIV20 1/4*/
  FDIV_MADD_MSUB(1,32)|FDCTRL_STRETCH,                  /*FDIV21 1/32*/
  FDIV_MADD_MSUB(1,2)|FDCTRL_STRETCH,                   /*FDIV22 1/2*/
  /*SPI_CLK_BASE*/
  FDIV_MADD_MSUB(1,2)|FDCTRL_STRETCH|FDCTRL_ENABLE,     /*FDIV23 1/2*/
};


static const Int32U ESR_Cnfg[SYSCLK_O+1-3] =
{
  /*SYS_BASE CLOCKS*/
  ESR_FDIV(FDIV0),ESR_FDIV(FDIV0), /*APB0_CLK, APB1_CLK*/
  ESR_FDIV(FDIV0),ESR_FDIV(FDIV0), /*APB2_CLK, APB3_CLK*/
  ESR_FDIV(FDIV0),ESR_FDIV(FDIV0), /*APB4_CLK, AHB_TO_INTC_CLK*/
  ESR_FDIV(FDIV0),ESR_FDIV(FDIV0), /*AHB0_CLK, EBI_CLK*/
  ESR_FDIV(FDIV2),ESR_FDIV(FDIV0), /*DMA_PCLK, DMA_CLK_GATED*/


  ESR_FDIV(FDIV0),ESR_FDIV(FDIV2), /*NANDFLASH_S0_CLK, NANDFLASH_ECC_CLK*/
   0,              ESR_FDIV(FDIV0), /*RESERVED, NANDFLASH_NAND_CLK*/
   ESR_FDIV(FDIV0),ESR_FDIV(FDIV0), /*NANDFLASH_PCLK, CLOCK_OUT*/
   BASE_CLOCK,     ESR_FDIV(FDIV0), /*ARM926_CORE_CLK,ARM926_BUSIF_CLKT*/
   BASE_CLOCK,     ESR_FDIV(FDIV0), /*ARM926_RETIME_CLK,SD_MMC_HCLK*/
   ESR_FDIV(FDIV0),ESR_FDIV(FDIV0), /*SD_MMC_CCLK_IN,USB_OTG_AHB_CLK*/
   ESR_FDIV(FDIV0),ESR_FDIV(FDIV2), /*ISRAM0_CLK,RED_CTL_RSCLK*/
   ESR_FDIV(FDIV0),ESR_FDIV(FDIV0), /*ISRAM1_CLK,ISROM_CLK*/

  // ESR_FDIV(FDIV0),ESR_FDIV(FDIV5), /*NANDFLASH_S0_CLK, NANDFLASH_ECC_CLK*/
  // 0,              ESR_FDIV(FDIV2), /*RESERVED, NANDFLASH_NAND_CLK*/
  // ESR_FDIV(FDIV2),ESR_FDIV(FDIV0), /*NANDFLASH_PCLK, CLOCK_OUT*/
  // BASE_CLOCK,     ESR_FDIV(FDIV0), /*ARM926_CORE_CLK,ARM926_BUSIF_CLKT*/
  // BASE_CLOCK,     ESR_FDIV(FDIV0), /*ARM926_RETIME_CLK,SD_MMC_HCLK*/
  // ESR_FDIV(FDIV0),ESR_FDIV(FDIV0), /*SD_MMC_CCLK_IN,USB_OTG_AHB_CLK*/
  // ESR_FDIV(FDIV0),ESR_FDIV(FDIV2), /*ISRAM0_CLK,RED_CTL_RSCLK*/
  // ESR_FDIV(FDIV0),ESR_FDIV(FDIV0), /*ISRAM1_CLK,ISROM_CLK*/
  ESR_FDIV(FDIV0),ESR_FDIV(FDIV0), /*MPMC_CFG_CLK,MPMC_CFG_CLK2*/
  BASE_CLOCK,     ESR_FDIV(FDIV0), /*MPMC_CFG_CLK3,INTC_CLK*/
  /*AHB_APB0_BASE CLOCKS*/
  ESR_FDIV(FDIV7-FDIV7),ESR_FDIV(FDIV7-FDIV7),/*AHB_TO_APB0_PCLK, EVENT_ROUTER_PCLK*/
  ESR_FDIV(FDIV7-FDIV7),ESR_FDIV(FDIV8-FDIV7),/*ADC_PCLK, ADC_CLK*/
  ESR_FDIV(FDIV7-FDIV7),ESR_FDIV(FDIV7-FDIV7),/*WDOG_PCLK,IOCONF_PCLK*/
  ESR_FDIV(FDIV7-FDIV7),ESR_FDIV(FDIV7-FDIV7),/*CGU_PCLK, SYSCREG_PCLK*/
  0,                    ESR_FDIV(FDIV7-FDIV7),/*RESERVED, RNG_PCLK*/
  /*AHB_APB1_BASE CLOCKS*/
  ESR_FDIV(FDIV9-FDIV9),ESR_FDIV(FDIV9-FDIV9), /*AHB_TO_APB1_PCLK, TIMER0_PCLK*/
  ESR_FDIV(FDIV9-FDIV9),ESR_FDIV(FDIV9-FDIV9), /*TIMER1_PCLK, TIMER2_PCLK*/
  ESR_FDIV(FDIV9-FDIV9),ESR_FDIV(FDIV9-FDIV9), /*TIMER3_PCLK, PWM_PCLK*/
  ESR_FDIV(FDIV9-FDIV9),ESR_FDIV(FDIV9-FDIV9), /*PWM_PCLK_REGS, PWM_CLK*/
  ESR_FDIV(FDIV9-FDIV9),ESR_FDIV(FDIV9-FDIV9), /*I2C0_PCLK, I2C1_PCLK*/
  /*AHB_APB2_BASE CLOCKS*/
  ESR_FDIV(FDIV11-FDIV11),ESR_FDIV(FDIV11-FDIV11),  /*AHB_TO_APB2_PCLK, PCM_PCLK*/
  ESR_FDIV(FDIV11-FDIV11),ESR_FDIV(FDIV11-FDIV11),  /*PCM_APB_PCLK, UART_APB_CLK*/
  ESR_FDIV(FDIV11-FDIV11),ESR_FDIV(FDIV12-FDIV11),  /*LCD_PCLK, LCD_CLK*/
  ESR_FDIV(FDIV11-FDIV11),ESR_FDIV(FDIV11-FDIV11),  /*SPI_PCLK, SPI_PCLK_GATED*/
  /*AHB_APB3_BASE CLOCKS*/
  BASE_CLOCK,           BASE_CLOCK,             /*AHB_TO_APB3_PCLK,I2S_CFG_PCLK*/
  BASE_CLOCK,           BASE_CLOCK,             /*EDGE_DET_PCLK, I2STX_FIFO_0_PCLK*/
  BASE_CLOCK,           BASE_CLOCK,             /*I2STX_IF_0_PCLK, I2STX_FIFO_1_PCLK*/
  BASE_CLOCK,           BASE_CLOCK,             /*I2STX_IF_1_PCLK, I2SRX_FIFO_0_PCLK*/
  BASE_CLOCK,           BASE_CLOCK,             /*I2SRX_IF_0_PCLK, I2SRX_FIFO_1_PCLK*/
  BASE_CLOCK,           0,                      /*2SRX_IF_1_PCLK, RESERVED*/
  0,                                            /*RESERVED*/
  /*PCM_BASE CLOCKS*/
  ESR_FDIV(FDIV15-FDIV15),  /*PCM_CLK_IP*/
  /*UART_BASE CLOCKS*/
  BASE_CLOCK,               /*UART_U_CLK*/
  /*CLK1024FS_BASE CLOCKS*/
  ESR_FDIV(FDIV17-FDIV17),ESR_FDIV(FDIV18-FDIV17),/*I2S_EDGE_DETECT_CLK, I2STX_BCK0_N*/
  ESR_FDIV(FDIV17-FDIV17),BASE_CLOCK,             /*I2STX_WS0, I2STX_CLK0*/
  ESR_FDIV(FDIV18-FDIV17),ESR_FDIV(FDIV17-FDIV17),/*I2STX_BCK1_N, I2STX_WS1*/
  BASE_CLOCK,             ESR_FDIV(FDIV20-FDIV17),/*CLK_256FS, I2SRX_BCK0_N*/
  ESR_FDIV(FDIV17-FDIV17),ESR_FDIV(FDIV20-FDIV17),/*I2SRX_WS0, I2SRX_BCK1_N*/
  ESR_FDIV(FDIV17-FDIV17),0,                      /*I2SRX_WS1, RESERVED*/
  0,                      0,                      /*RESERVED, RESERVED*/
  /*SPI_CLK_BASE CLOCKS*/
  ESR_FDIV(FDIV23-FDIV23),ESR_FDIV(FDIV23-FDIV23)/*SPI_CLK, SPI_CLK_GATED*/
};

/** public functions **/

void CGU_Run_Clock(CGU_CLOCKS Clock)
{
__pcr_bits * pcr = (__pcr_bits *)(&PCR0+Clock);
  pcr->RUN = 1;
}

void CGU_Stop_Clock(CGU_CLOCKS Clock)
{
__pcr_bits * pcr = (__pcr_bits *)(&PCR0+Clock);
  pcr->RUN = 0;
}

/*************************************************************************
 * Function Name: InitClock
 * Parameters: None
 *
 * Return: None
 *
 * Description:
 *
 *************************************************************************/
void InitClock(void)
{
DOMAIN DomainID;
CGU_CLOCKS ClockID;

  for(DomainID = DMN_SYS; DMN_SYSCLK_O >= DomainID; DomainID++)
  {
    /*Switch all clock domains to 12MHz*/
    CGU_SelectDomainClk(DomainID,CLK_FFAST);
    /*Disable Fdiv in BCR*/
    CGU_CnfgBCR(DomainID, 0x0);
  }

  for(ClockID = APB0_CLK ; SYSCLK_O >= ClockID; ClockID++)
  {
    /*this clocks don't exist*/
    if((12 == ClockID) || (38 == ClockID) || (69 == ClockID) || (70 == ClockID) ||\
       (84 == ClockID) || (85 == ClockID) || (86 == ClockID)) continue;

    /*Clear ESR registers*/
    if((I2SRX_BCK0 != ClockID) && (I2SRX_BCK1 != ClockID) && (SYSCLK_O != ClockID))
    {
      if(I2SRX_BCK0 >ClockID) *((Int32U *)&ESR0 +ClockID) = 0;
      else *((Int32U *)&ESR0 +ClockID-2) = 0;
    }
    /*Enable extern*/
    switch(ClockID)
    {
      case DMA_CLK_GATED:
      case EVENT_ROUTER_PCLK:
      case ADC_PCLK:
      case IOCONF_PCLK:
      case CGU_PCLK:
      case SYSCREG_PCLK:
      case PWM_PCLK_REGS:
      case PCM_APB_PCLK:
      case SPI_PCLK_GATED:
      case PCM_CLK_IP:
      case SPI_CLK_GATED:
        *((Int32U *)&PCR0 + ClockID) |= (0x1<<3);
      break;
      case WDOG_PCLK:
      case I2C0_PCLK:
      case I2C1_PCLK:
      case UART_APB_CLK:
      case LCD_PCLK:
        *((Int32U *)&PCR0 + ClockID) &= ~(0x1<<3);
      break;
      default:
      break;
    }

    /*Enable Out*/
    if((ARM926_BUSIF_CLK == ClockID) || (MPMC_CFG_CLK == ClockID))
    {
        *((Int32U *)&PCR0 + ClockID) |= (0x1<<4);
    }
    else
    {
        *((Int32U *)&PCR0 + ClockID) &= ~(0x1<<4);
    }

    /*Check default enabled clocks*/
    if(((32> ClockID) && (CLK_EN_0_31 & (1<<ClockID))) || \
       ((64> ClockID) && (CLK_EN_32_63 & (1<<(ClockID-32)))) ||\
       ((CLK_EN_64_92 & (1<<(ClockID-64)))))
    {
        /*Enable Clock*/
        CGU_Run_Clock(ClockID);
    }
    else
    {   /*Disable Clock*/
        CGU_Stop_Clock(ClockID);
    }
  }

  /*PLL Init 180MHz*/
  HP1_MODE = 0x4;     /*Power Down PLL*/

  HP1_FIN_SELECT = 0; /*Select FFAST as Pll input clock*/
  HP1_MDEC = 8191;    /*M devider*/
  HP1_NDEC = 770;     /*N devider*/
  HP1_PDEC = 98;      /*P devider*/

  HP1_SELR = 0;       /**/
  HP1_SELI = 16;
  HP1_SELP = 8;

  HP1_MODE = 1;             /*Enable PLL*/

  while(!(HP1_STATUS & 1)); /*Wait untill PLL locks*/

  /*Configure the deviders*/
  for(int i = FDIV0; FDIV23 >= i ; i++)
  {
    /*Clear Run Bit*/
    *((Int32U *)&FDC0 + i) &= ~(0x1<<0);
    /*Configure Devider*/
    *((Int32U *)&FDC0 + i) = FDIV_Config[i];
  }

  for(ClockID = APB0_CLK; SYSCLK_O >= ClockID; ClockID++)
  {
    /*this clocks don't exist*/
    if((12 == ClockID) || (38 == ClockID) || (69 == ClockID) || (70 == ClockID) ||\
       (84 == ClockID) || (85 == ClockID) || (86 == ClockID)) continue;

    /*Init ESR registers*/
    if((I2SRX_BCK0 != ClockID) && (I2SRX_BCK1 != ClockID) && (SYSCLK_O != ClockID))
    {
      if(I2SRX_BCK0 >ClockID) *((Int32U *)&ESR0 +ClockID) = ESR_Cnfg[ClockID];
      else *((Int32U *)&ESR0 +ClockID-2) = ESR_Cnfg[ClockID-2];
    }
  }

  /*Enable SYS_BASE Deviders*/
  CGU_CnfgBCR(DMN_SYS, 0x1);
  /*PLL1 as SYS_BASE clock*/
  CGU_SelectDomainClk(DMN_SYS,CLK_HPPLL1);
  /*Enable DMN_AHB_APB0 Deviders*/
  CGU_CnfgBCR(DMN_AHB_APB0, 0x1);
  /*PLL1 as DMN_AHB_APB0 clock*/
  CGU_SelectDomainClk(DMN_AHB_APB0,CLK_HPPLL1);
  /*Enable DMN_AHB_APB1 Deviders*/
  CGU_CnfgBCR(DMN_AHB_APB1, 0x1);
  /*CLK_FFAST as DMN_AHB_APB1 clock*/
  CGU_SelectDomainClk(DMN_AHB_APB1,CLK_FFAST);
  /*Enable DMN_AHB_APB2 Deviders*/
  CGU_CnfgBCR(DMN_AHB_APB2, 0x1);
  /*PLL1 as DMN_AHB_APB2 clock*/
  CGU_SelectDomainClk(DMN_AHB_APB2,CLK_HPPLL1);
  /*CLK_FFAST as DMN_AHB_APB3 clock*/
  CGU_SelectDomainClk(DMN_AHB_APB3,CLK_FFAST);
  /*PLL1 as DMN_PCM clock*/
  CGU_SelectDomainClk(DMN_PCM,CLK_HPPLL1);
  /*CLK_FFAST as DMN_UART clock*/
  CGU_SelectDomainClk(DMN_UART,CLK_FFAST);
  /*Enable DMN_CLK1024FS Deviders*/
  CGU_CnfgBCR(DMN_CLK1024FS, 0x1);
  /*CLK_HPPLL0 as DMN_CLK1024FS clock*/
  CGU_SelectDomainClk(DMN_CLK1024FS,CLK_HPPLL0);
  /*CLK_I2SRX_BCK0 as DMN_I2SRX_BCK0 clock*/
  CGU_SelectDomainClk(DMN_I2SRX_BCK0,CLK_I2SRX_BCK0);
  /*CLK_I2SRX_BCK1 as DMN_I2SRX_BCK1 clock*/
  CGU_SelectDomainClk(DMN_I2SRX_BCK1,CLK_I2SRX_BCK1);
  /*PLL1 as DMN_SPI_CLK clock*/
  CGU_SelectDomainClk(DMN_SPI_CLK,CLK_HPPLL1);
  /*CLK_FFAST as DMN_SYSCLK_O clock*/
  CGU_SelectDomainClk(DMN_SYSCLK_O,CLK_FFAST);
}

/** private functions **/
void CGU_SelectDomainClk(DOMAIN Domain, IN_CLOCKS Clock)
{
  switch (*((Int32U *)&SSR0 + Domain) & 0x3)
  {
  case 0:
  case 3:
  /*Reset Switch*/
      *((Int32U *)&SCR0 + Domain) = 0x4;
  case 2:
    /*Select Clock*/
    *((Int32U *)&FS1_0 + Domain) = Clock;
    /*Select switch 1*/
    *((Int32U *)&SCR0 + Domain) = 0x1;

    break;
  case 1:
    /*Select Clock*/
    *((Int32U *)&FS2_0 + Domain) = Clock;
    /*Select switch 2*/
    *((Int32U *)&SCR0 + Domain) = 0x2;
    break;
  }
}

void CGU_CnfgBCR(DOMAIN Domain,Int32U Cnfg)
{
  if(DMN_AHB_APB2 >= Domain) *((Int32U *)&BCR0 + Domain) = Cnfg;
  if(DMN_CLK1024FS == Domain) BCR7 = Cnfg;
}

