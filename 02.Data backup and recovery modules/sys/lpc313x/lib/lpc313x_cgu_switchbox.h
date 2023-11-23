/***********************************************************************
 * $Id:: lpc313x_cgu_switchbox.h 1757 2009-03-27 00:11:10Z pdurgesh    $
 *
 * Project: LPC313X family chip definitions
 *
 * Description:
 *     This file contains the structure definitions and manifest
 *     constants for the LPC313X chip family component:
 *         Clock generation and switchbox controller
 *
 * Notes:
 *
 ***********************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
 **********************************************************************/

#ifndef LPC313X_CGU_SWITCHBOX_H
#define LPC313X_CGU_SWITCHBOX_H

/***********************************************************************
* Include files
***********************************************************************/

#include "lpc_types.h"
#include "lpc313x_chip.h"
#include "lpc313x_cgu.h"

#ifdef __cplusplus
extern "C"    /* Assume C declarations for C++ */
{
#endif

   /***********************************************************************
   * Useful values
   ***********************************************************************/
#define CGU_SB_FSR_WIDTH   3
#define CGU_SB_NR_BASE     12
#define CGU_SB_NR_CLK      92
#define CGU_SB_NR_BCR      5
#define CGU_SB_NR_FRACDIV  24
#define CGU_SB_NR_DYN_FDIV 7
#define CGU_SB_NR_ESR      89


   /***********************************************************************
    Register section offsets in APB address space
   ***********************************************************************/
#define SCR_OFFSET 0
#define FS1_OFFSET 12
#define FS2_OFFSET 24
#define SSR_OFFSET 36
#define PCR_OFFSET 48
#define PSR_OFFSET 140
#define ESR_OFFSET 232
#define BCR_OFFSET 321
#define FDC_OFFSET 326


   /***********************************************************************
   * CGU Switchbox register structure
   **********************************************************************/
   typedef volatile struct
   {
      /* Switches controls */
      volatile UNS_32 base_scr[12]; /* Switch control */
      volatile UNS_32 base_fs1[12]; /* Frequency select side 1 */
      volatile UNS_32 base_fs2[12]; /* Frequency select side 2 */
      volatile UNS_32 base_ssr[12]; /* Switch status */
      /* Clock enable controls (positive and inverted clock pairs share
      control register)*/
      volatile UNS_32 clk_pcr[92];  /* power control */
      volatile UNS_32 clk_psr[92];  /* power status */
      /* enable select from fractional dividers (positive and inverted
      clock pairs share esr)*/
      volatile UNS_32 clk_esr[89];  /* enable select */
      /* Base controls, currently only fd_run (base wide fractional
      divider enable) bit.*/
      volatile UNS_32 base_bcr[5];  /* Base control */
      /* Fractional divider controls & configuration*/
      volatile UNS_32 base_fdc[24]; /* Fractional divider config & ctrl */
      volatile UNS_32 base_dyn_fdc[7]; /* Fractional divider config & ctrl
                                   for dynamic fracdivs */
      volatile UNS_32 base_dyn_sel[7]; /* Fractional divider register for
                                   selecting an external signal to
                                   trigger high-speed operation */
   } CGU_SB_REGS_T;

   /***********************************************************************
   * Bit positions
   ***********************************************************************/
   /* Switch Control Register */
#define CGU_SB_SCR_EN1              _BIT(0)
#define CGU_SB_SCR_EN2              _BIT(1)
#define CGU_SB_SCR_RST              _BIT(2)
#define CGU_SB_SCR_STOP             _BIT(3)
#define CGU_SB_SCR_FS_MASK          0x3

   /* Switch Status Register */
#define CGU_SB_SSR_FS_GET(x)        ( ((x) >> 2) & 0x7)
   /* Power Control Register */
#define CGU_SB_PCR_RUN              _BIT(0)
#define CGU_SB_PCR_AUTO             _BIT(1)
#define CGU_SB_PCR_WAKE_EN          _BIT(2)
#define CGU_SB_PCR_EXTEN_EN         _BIT(3)
#define CGU_SB_PCR_ENOUT_EN         _BIT(4)
   /* Power Status Register */
#define CGU_SB_PSR_ACTIVE           _BIT(0)
#define CGU_SB_PSR_WAKEUP           _BIT(1)
   /* Enable Select Register */
#define CGU_SB_ESR_ENABLE           _BIT(0)
#define CGU_SB_ESR_SELECT(x)        _SBF(1, (x))
#define CGU_SB_ESR_SEL_GET(x)       (((x) >> 1) & 0x7)

   /* Base control Register */
#define CGU_SB_BCR_FD_RUN           _BIT(0)
   /* Fractional Divider Configuration Register */
#define CGU_SB_FDC_RUN              _BIT(0)
#define CGU_SB_FDC_RESET            _BIT(1)
#define CGU_SB_FDC_STRETCH          _BIT(2)
#define CGU_SB_FDC_MADD(x)          _SBF( 3, ((x) & 0xFF))
#define CGU_SB_FDC_MSUB(x)          _SBF(11, ((x) & 0xFF))
#define CGU_SB_FDC17_MADD(x)        _SBF( 3, ((x) & 0x1FFF))
#define CGU_SB_FDC17_MSUB(x)        _SBF(16, ((x) & 0x1FFF))
#define CGU_SB_FDC_MADD_GET(x)      (((x) >> 3) & 0xFF)
#define CGU_SB_FDC_MSUB_GET(x)      ((((x) >> 11) & 0xFF) | 0xFFFFFF00)
#define CGU_SB_FDC17_MADD_GET(x)    (((x) >> 3) & 0x1FFF)
#define CGU_SB_FDC17_MSUB_GET(x)    ((((x) >> 16) & 0x1FFF) | 0xFFFFE000)
#define CGU_SB_FDC_MADD_POS         3

   /* Dynamic Fractional Divider Configuration Register */
#define CGU_SB_DYN_FDC_RUN          _BIT(0)
#define CGU_SB_DYN_FDC_ALLOW        _BIT(1)
#define CGU_SB_DYN_FDC_STRETCH      _BIT(2)

   /***********************************************************************
   * Clock domain base id's
   ***********************************************************************/
   typedef enum
   {
      CGU_SB_SYS_BASE_ID = 0,
      CGU_SB_BASE_FIRST = CGU_SB_SYS_BASE_ID,
      CGU_SB_AHB0_APB0_BASE_ID,
      CGU_SB_AHB0_APB1_BASE_ID,
      CGU_SB_AHB0_APB2_BASE_ID,
      CGU_SB_AHB0_APB3_BASE_ID,
      CGU_SB_PCM_BASE_ID,
      CGU_SB_UART_BASE_ID,
      CGU_SB_CLK1024FS_BASE_ID,
      CGU_SB_I2SRX_BCK0_BASE_ID,
      CGU_SB_I2SRX_BCK1_BASE_ID,
      CGU_SB_SPI_CLK_BASE_ID,
      CGU_SB_SYSCLK_O_BASE_ID,
      CGU_SB_BASE_LAST = CGU_SB_SYSCLK_O_BASE_ID
   } CGU_DOMAIN_ID_T;

   /***********************************************************************
    Clock id's (= clkid in address calculation)
   ***********************************************************************/
   typedef enum
   {
      /* domain 0 = SYS_BASE */
      CGU_SB_APB0_CLK_ID = 0,
      CGU_SYS_FIRST = CGU_SB_APB0_CLK_ID,
      CGU_SB_APB1_CLK_ID,
      CGU_SB_APB2_CLK_ID,
      CGU_SB_APB3_CLK_ID,
      CGU_SB_APB4_CLK_ID,
      CGU_SB_AHB2INTC_CLK_ID,
      CGU_SB_AHB0_CLK_ID,
      CGU_SB_EBI_CLK_ID,
      CGU_SB_DMA_PCLK_ID,
      CGU_SB_DMA_CLK_GATED_ID,
      CGU_SB_NANDFLASH_S0_CLK_ID,
      CGU_SB_NANDFLASH_ECC_CLK_ID,
      CGU_SB_NANDFLASH_AES_CLK_ID, /* valid on LPC3153 & LPC3154 only */
      CGU_SB_NANDFLASH_NAND_CLK_ID,
      CGU_SB_NANDFLASH_PCLK_ID,
      CGU_SB_CLOCK_OUT_ID,
      CGU_SB_ARM926_CORE_CLK_ID,
      CGU_SB_ARM926_BUSIF_CLK_ID,
      CGU_SB_ARM926_RETIME_CLK_ID,
      CGU_SB_SD_MMC_HCLK_ID,
      CGU_SB_SD_MMC_CCLK_IN_ID,
      CGU_SB_USB_OTG_AHB_CLK_ID,
      CGU_SB_ISRAM0_CLK_ID,
      CGU_SB_RED_CTL_RSCLK_ID,
      CGU_SB_ISRAM1_CLK_ID,
      CGU_SB_ISROM_CLK_ID,
      CGU_SB_MPMC_CFG_CLK_ID,
      CGU_SB_MPMC_CFG_CLK2_ID,
      CGU_SB_MPMC_CFG_CLK3_ID,
      CGU_SB_INTC_CLK_ID,
      CGU_SYS_LAST = CGU_SB_INTC_CLK_ID,

      /* domain 1 = AHB0APB0_BASE */
      CGU_SB_AHB2APB0_ASYNC_PCLK_ID,
      CGU_AHB0APB0_FIRST = CGU_SB_AHB2APB0_ASYNC_PCLK_ID,
      CGU_SB_EVENT_ROUTER_PCLK_ID,
      CGU_SB_ADC_PCLK_ID,
      CGU_SB_ADC_CLK_ID,
      CGU_SB_WDOG_PCLK_ID,
      CGU_SB_IOCONF_PCLK_ID,
      CGU_SB_CGU_PCLK_ID,
      CGU_SB_SYSCREG_PCLK_ID,
      CGU_SB_OTP_PCLK_ID, /* valid on LPC315x series only */
      CGU_SB_RNG_PCLK_ID,
      CGU_AHB0APB0_LAST = CGU_SB_RNG_PCLK_ID,


      /* domain 2 = AHB0APB1_BASE */
      CGU_SB_AHB2APB1_ASYNC_PCLK_ID,
      CGU_AHB0APB1_FIRST = CGU_SB_AHB2APB1_ASYNC_PCLK_ID,
      CGU_SB_TIMER0_PCLK_ID,
      CGU_SB_TIMER1_PCLK_ID,
      CGU_SB_TIMER2_PCLK_ID,
      CGU_SB_TIMER3_PCLK_ID,
      CGU_SB_PWM_PCLK_ID,
      CGU_SB_PWM_PCLK_REGS_ID,
      CGU_SB_PWM_CLK_ID,
      CGU_SB_I2C0_PCLK_ID,
      CGU_SB_I2C1_PCLK_ID,
      CGU_AHB0APB1_LAST = CGU_SB_I2C1_PCLK_ID,

      /* domain 3 = AHB0APB2_BASE */
      CGU_SB_AHB2APB2_ASYNC_PCLK_ID,
      CGU_AHB0APB2_FIRST = CGU_SB_AHB2APB2_ASYNC_PCLK_ID,
      CGU_SB_PCM_PCLK_ID,
      CGU_SB_PCM_APB_PCLK_ID,
      CGU_SB_UART_APB_CLK_ID,
      CGU_SB_LCD_PCLK_ID,
      CGU_SB_LCD_CLK_ID,
      CGU_SB_SPI_PCLK_ID,
      CGU_SB_SPI_PCLK_GATED_ID,
      CGU_AHB0APB2_LAST = CGU_SB_SPI_PCLK_GATED_ID,

      /* domain 4 = AHB0APB3_BASE */
      CGU_SB_AHB2APB3_ASYNC_PCLK_ID,
      CGU_AHB0APB3_FIRST = CGU_SB_AHB2APB3_ASYNC_PCLK_ID,
      CGU_SB_I2S_CFG_PCLK_ID,
      CGU_SB_EDGE_DET_PCLK_ID,
      CGU_SB_I2STX_FIFO_0_PCLK_ID,
      CGU_SB_I2STX_IF_0_PCLK_ID,
      CGU_SB_I2STX_FIFO_1_PCLK_ID,
      CGU_SB_I2STX_IF_1_PCLK_ID,
      CGU_SB_I2SRX_FIFO_0_PCLK_ID,
      CGU_SB_I2SRX_IF_0_PCLK_ID,
      CGU_SB_I2SRX_FIFO_1_PCLK_ID,
      CGU_SB_I2SRX_IF_1_PCLK_ID,
      CGU_SB_RSVD69_ID,
      CGU_SB_AHB2APB3_RSVD_ID,
      CGU_AHB0APB3_LAST = CGU_SB_AHB2APB3_RSVD_ID,

      /* domain 5 = PCM_BASE */
      CGU_SB_PCM_CLK_IP_ID,
      CGU_PCM_FIRST = CGU_SB_PCM_CLK_IP_ID,
      CGU_PCM_LAST = CGU_SB_PCM_CLK_IP_ID,

      /* domain 6 = UART_BASE */
      CGU_SB_UART_U_CLK_ID,
      CGU_UART_FIRST = CGU_SB_UART_U_CLK_ID,
      CGU_UART_LAST = CGU_SB_UART_U_CLK_ID,

      /* domain 7 = CLK1024FS_BASE */
      CGU_SB_I2S_EDGE_DETECT_CLK_ID,
      CGU_CLK1024FS_FIRST = CGU_SB_I2S_EDGE_DETECT_CLK_ID,
      CGU_SB_I2STX_BCK0_N_ID,
      CGU_SB_I2STX_WS0_ID,
      CGU_SB_I2STX_CLK0_ID,
      CGU_SB_I2STX_BCK1_N_ID,
      CGU_SB_I2STX_WS1_ID,
      CGU_SB_CLK_256FS_ID,
      CGU_SB_I2SRX_BCK0_N_ID,
      CGU_SB_I2SRX_WS0_ID,
      CGU_SB_I2SRX_BCK1_N_ID,
      CGU_SB_I2SRX_WS1_ID,
      CGU_SB_RSVD84_ID,
      CGU_SB_RSVD85_ID,
      CGU_SB_RSVD86_ID,
      CGU_CLK1024FS_LAST = CGU_SB_RSVD86_ID,

      /* domain 8 = BCK0_BASE */
      CGU_SB_I2SRX_BCK0_ID,
      CGU_I2SRX_BCK0_FIRST = CGU_SB_I2SRX_BCK0_ID,
      CGU_I2SRX_BCK0_LAST = CGU_SB_I2SRX_BCK0_ID,

      /* domain 9 = BCK1_BASE */
      CGU_SB_I2SRX_BCK1_ID,
      CGU_I2SRX_BCK1_FIRST = CGU_SB_I2SRX_BCK1_ID,
      CGU_I2SRX_BCK1_LAST = CGU_SB_I2SRX_BCK1_ID,

      /* domain 10 = SPI_BASE */
      CGU_SB_SPI_CLK_ID,
      CGU_SPI_FIRST = CGU_SB_SPI_CLK_ID,
      CGU_SB_SPI_CLK_GATED_ID,
      CGU_SPI_LAST = CGU_SB_SPI_CLK_GATED_ID,

      /* domain 11 = SYSCLKO_BASE */
      CGU_SB_SYSCLK_O_ID,
      CGU_SYSCLK_O_FIRST = CGU_SB_SYSCLK_O_ID,
      CGU_SYSCLK_O_LAST = CGU_SB_SYSCLK_O_ID,

      CGU_SB_INVALID_CLK_ID = -1
   } CGU_CLOCK_ID_T;

   /***********************************************************************
   * NR of fractional dividers available for each base frequency,
   * their bit widths and extractions for sub elements from the
   * fractional divider configuration register
   ***********************************************************************/
#define CGU_SB_BASE0_FDIV_CNT           7
#define CGU_SB_BASE0_FDIV_LOW_ID        0
#define CGU_SB_BASE0_FDIV_HIGH_ID       6
#define CGU_SB_BASE0_FDIV0_W            8

#define CGU_SB_BASE1_FDIV_CNT           2
#define CGU_SB_BASE1_FDIV_LOW_ID        7
#define CGU_SB_BASE1_FDIV_HIGH_ID       8
#define CGU_SB_BASE1_FDIV0_W            8

#define CGU_SB_BASE2_FDIV_CNT           2
#define CGU_SB_BASE2_FDIV_LOW_ID        9
#define CGU_SB_BASE2_FDIV_HIGH_ID       10
#define CGU_SB_BASE2_FDIV0_W            8

#define CGU_SB_BASE3_FDIV_CNT           3
#define CGU_SB_BASE3_FDIV_LOW_ID        11
#define CGU_SB_BASE3_FDIV_HIGH_ID       13
#define CGU_SB_BASE3_FDIV0_W            8

#define CGU_SB_BASE4_FDIV_CNT           1
#define CGU_SB_BASE4_FDIV_LOW_ID        14
#define CGU_SB_BASE4_FDIV_HIGH_ID       14
#define CGU_SB_BASE4_FDIV0_W            8

#define CGU_SB_BASE5_FDIV_CNT           1
#define CGU_SB_BASE5_FDIV_LOW_ID        15
#define CGU_SB_BASE5_FDIV_HIGH_ID       15
#define CGU_SB_BASE5_FDIV0_W            8

#define CGU_SB_BASE6_FDIV_CNT           1
#define CGU_SB_BASE6_FDIV_LOW_ID        16
#define CGU_SB_BASE6_FDIV_HIGH_ID       16
#define CGU_SB_BASE6_FDIV0_W            8

#define CGU_SB_BASE7_FDIV_CNT           6
#define CGU_SB_BASE7_FDIV_LOW_ID        17
#define CGU_SB_BASE7_FDIV_HIGH_ID       22
#define CGU_SB_BASE7_FDIV0_W            13

#define CGU_SB_BASE8_FDIV_CNT           0
#define CGU_SB_BASE9_FDIV_CNT           0
#define CGU_SB_BASE10_FDIV_CNT          1
#define CGU_SB_BASE10_FDIV_LOW_ID       23
#define CGU_SB_BASE10_FDIV_HIGH_ID      23
#define CGU_SB_BASE10_FDIV0_W           8

#define CGU_SB_BASE11_FDIV_CNT          0


   /* Macro pointing to CGU switch box registers */
#define CGU_SB  ((CGU_SB_REGS_T  *)(CGU_SWITCHBOX_BASE))

#ifdef __cplusplus
}
#endif

#endif /* LPC313X_CGU_SB_H */
