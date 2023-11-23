/*************************************************************************
 *
 *   Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2008
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
 **************************************************************************/

/** include files **/
#include <NXP\iolpc3130.h>
#include "arm_comm.h"
/** definitions **/
typedef enum
{
   CLK_FFAST = 0,
   CLK_I2SRX_BCK0,
   CLK_I2SRX_WS0,
   CLK_I2SRX_BCK1,
   CLK_I2SRX_WS1,
   CLK_HPPLL0,
   CLK_HPPLL1
} IN_CLOCKS;

typedef enum
{
   DMN_SYS = 0,
   DMN_AHB_APB0,
   DMN_AHB_APB1,
   DMN_AHB_APB2,
   DMN_AHB_APB3,
   DMN_PCM,
   DMN_UART,
   DMN_CLK1024FS,
   DMN_I2SRX_BCK0,
   DMN_I2SRX_BCK1,
   DMN_SPI_CLK,
   DMN_SYSCLK_O
} DOMAIN;

typedef enum
{
   /*SYS_BASE CLOCKS*/
   APB0_CLK = 0, APB1_CLK, APB2_CLK, APB3_CLK, APB4_CLK,
   AHB_TO_INTC_CLK, AHB0_CLK, EBI_CLK, DMA_PCLK, DMA_CLK_GATED,
   NANDFLASH_S0_CLK, NANDFLASH_ECC_CLK, NANDFLASH_NAND_CLK = 13,
   NANDFLASH_PCLK, CLOCK_OUT, ARM926_CORE_CLK, ARM926_BUSIF_CLK,
   ARM926_RETIME_CLK, SD_MMC_HCLK, SD_MMC_CCLK_IN, USB_OTG_AHB_CLK,
   ISRAM0_CLK, RED_CTL_RSCLK, ISRAM1_CLK, ISROM_CLK, MPMC_CFG_CLK,
   MPMC_CFG_CLK2, MPMC_CFG_CLK3, INTC_CLK,
   /*AHB_APB0_BASE CLOCKS*/
   AHB_TO_APB0_PCLK, EVENT_ROUTER_PCLK, ADC_PCLK, ADC_CLK, WDOG_PCLK,
   IOCONF_PCLK, CGU_PCLK, SYSCREG_PCLK, RNG_PCLK = 39,
   /*AHB_APB1_BASE CLOCKS*/
   AHB_TO_APB1_PCLK, TIMER0_PCLK, TIMER1_PCLK, TIMER2_PCLK, TIMER3_PCLK,
   PWM_PCLK, PWM_PCLK_REGS, PWM_CLK, I2C0_PCLK, I2C1_PCLK,
   /*AHB_APB2_BASE*/
   AHB_TO_APB2_PCLK, PCM_PCLK, PCM_APB_PCLK, UART_APB_CLK, LCD_PCLK, LCD_CLK,
   SPI_PCLK, SPI_PCLK_GATED,
   /*AHB_APB3_BASE*/
   AHB_TO_APB3_PCLK, I2S_CFG_PCLK, EDGE_DET_PCLK, I2STX_FIFO_0_PCLK,
   I2STX_IF_0_PCLK, I2STX_FIFO_1_PCLK, I2STX_IF_1_PCLK, I2SRX_FIFO_0_PCLK,
   I2SRX_IF_0_PCLK, I2SRX_FIFO_1_PCLK, I2SRX_IF_1_PCLK,
   /*PCM_BASE CLOCKS*/
   PCM_CLK_IP = 71,
   /*UART_BASE CLOCKS*/
   UART_U_CLK,
   /*CLK1024FS_BASE*/
   I2S_EDGE_DETECT_CLK, I2STX_BCK0_N, I2STX_WS0, I2STX_CLK0, I2STX_BCK1_N,
   I2STX_WS1, CLK_256FS, I2SRX_BCK0_N, I2SRX_WS0, I2SRX_BCK1_N, I2SRX_WS1,
   /*I2SRX_BCK0_BASE CLOCKS*/
   I2SRX_BCK0 = 87,
   /*I2SRX_BCK1_BASE CLOCKS*/
   I2SRX_BCK1,
   /*SPI_CLK_BASE CLOCKS*/
   SPI_CLK,SPI_CLK_GATED,
   /*SYSCLK_O_BASE CLOCKS*/
   SYSCLK_O
} CGU_CLOCKS ;

typedef enum
{
   FDIV0 = 0, FDIV1, FDIV2,  FDIV3,  FDIV4,  FDIV5,  FDIV6,  FDIV7,
   FDIV8, FDIV9, FDIV10, FDIV11, FDIV12, FDIV13, FDIV14, FDIV15,
   FDIV16, FDIV17, FDIV18, FDIV19, FDIV20, FDIV21, FDIV22, FDIV23,
} FDIV;

/** public data **/

/** public functions **/
void CGU_Run_Clock(CGU_CLOCKS Clock);
void CGU_Stop_Clock(CGU_CLOCKS Clock);

/*************************************************************************
 * Function Name: InitClock
 * Parameters: None
 *
 * Return: None
 *
 * Description:
 *
 *************************************************************************/
void InitClock(void);
