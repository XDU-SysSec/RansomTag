/***********************************************************************
 * $Id:: lpc313x_cgu_driver.h 3557 2010-05-20 00:31:30Z usb10131       $
 *
 * Project: LPC313X CGU driver
 *
 * Description:
 *     This file contains driver support for the CGU module on the
 *     LPC313X.
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

#ifndef LPC313X_CGU_DRIVER_H
#define LPC313X_CGU_DRIVER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lpc313x_cgu.h"
#include "lpc313x_cgu_switchbox.h"

   /***********************************************************************
   * CGU driver defines - MACROS & constants
   **********************************************************************/
#define CGU_INVALID_ID  0xFFFF

   /* Following clocks are enabled after init.
   CGU_DEF_CLKS_0_31 contains bits for clocks with id between 0 & 31
   CGU_DEF_CLKS_32_63 contains bits for clocks with id between 32 & 63
   CGU_DEF_CLKS_64_92 contains bits for clocks with id between 64 & 92
   */
#define CGU_DEF_CLKS_0_31   ( _BIT(CGU_SB_APB0_CLK_ID) | _BIT(CGU_SB_APB1_CLK_ID) | \
                              _BIT(CGU_SB_APB2_CLK_ID) | _BIT(CGU_SB_APB3_CLK_ID) |_BIT(CGU_SB_APB4_CLK_ID) | \
                              _BIT(CGU_SB_AHB2INTC_CLK_ID) | _BIT(CGU_SB_AHB0_CLK_ID) | \
                              _BIT(CGU_SB_ARM926_CORE_CLK_ID) | _BIT(CGU_SB_ARM926_BUSIF_CLK_ID) | \
                              _BIT(CGU_SB_ARM926_RETIME_CLK_ID) | _BIT(CGU_SB_ISRAM0_CLK_ID) | \
                              _BIT(CGU_SB_ISRAM1_CLK_ID) | _BIT(CGU_SB_ISROM_CLK_ID) | \
                              _BIT(CGU_SB_INTC_CLK_ID) | _BIT(CGU_SB_AHB2APB0_ASYNC_PCLK_ID) | \
                              _BIT(CGU_SB_EVENT_ROUTER_PCLK_ID))

#define CGU_DEF_CLKS_32_63 ( _BIT(CGU_SB_IOCONF_PCLK_ID - 32) | _BIT(CGU_SB_CGU_PCLK_ID - 32) | \
                             _BIT(CGU_SB_SYSCREG_PCLK_ID - 32) | _BIT(CGU_SB_OTP_PCLK_ID - 32) | \
                             _BIT(CGU_SB_AHB2APB1_ASYNC_PCLK_ID - 32) | _BIT(CGU_SB_AHB2APB2_ASYNC_PCLK_ID - 32) | \
                             _BIT(CGU_SB_AHB2APB3_ASYNC_PCLK_ID - 32) )

#define CGU_DEF_CLKS_64_92 (0)

   /* Following macros are used to define clocks belonging to different
      sub-domains with-in each domain. */
#define D0_BIT(clkid)   _BIT(clkid)
#define D1_BIT(clkid)   _BIT((clkid) - CGU_AHB0APB0_FIRST)
#define D2_BIT(clkid)   _BIT((clkid) - CGU_AHB0APB1_FIRST)
#define D3_BIT(clkid)   _BIT((clkid) - CGU_AHB0APB2_FIRST)
#define D4_BIT(clkid)   _BIT((clkid) - CGU_AHB0APB3_FIRST)
#define D5_BIT(clkid)   _BIT((clkid) - CGU_PCM_FIRST)
#define D6_BIT(clkid)   _BIT((clkid) - CGU_UART_FIRST)
#define D7_BIT(clkid)   _BIT((clkid) - CGU_CLK1024FS_FIRST)
   /* 8 & 9 have one clk per domain so no macros */
#define D10_BIT(clkid)  _BIT((clkid) - CGU_SPI_FIRST)


   /***********************************************************************
   * CGU driver enumerations
   **********************************************************************/
   /* Possible HPLL ids */
   typedef enum {
      CGU_HPLL0_ID, CGU_HPLL1_ID
   }
   CGU_HPLL_ID_T;

   /* CGU soft reset module ID enumerations */
   typedef enum
   {
      APB0_RST_SOFT = 0,
      AHB2APB0_PNRES_SOFT,
      APB1_RST_SOFT,
      AHB2APB1_PNRES_SOFT,
      APB2_RESETN_SOFT,
      AHB2APB2_PNRES_SOFT,
      APB3_RESETN_SOFT,
      AHB2APB3_PNRES_SOFT,
      APB4_RESETN_SOFT,
      AHB2INTC_RESETN_SOFT,
      AHB0_RESETN_SOFT,
      EBI_RESETN_SOFT,
      PCM_PNRES_SOFT,
      PCM_RESET_N_SOFT,
      PCM_RESET_ASYNC_N_SOFT,
      TIMER0_PNRES_SOFT,
      TIMER1_PNRES_SOFT,
      TIMER2_PNRES_SOFT,
      TIMER3_PNRES_SOFT,
      ADC_PRESETN_SOFT,
      ADC_RESETN_ADC10BITS_SOFT,
      PWM_RESET_AN_SOFT,
      UART_SYS_RST_AN_SOFT,
      I2C0_PNRES_SOFT,
      I2C1_PNRES_SOFT,
      I2S_CFG_RST_N_SOFT,
      I2S_NSOF_RST_N_SOFT,
      EDGE_DET_RST_N_SOFT,
      I2STX_FIFO_0_RST_N_SOFT,
      I2STX_IF_0_RST_N_SOFT,
      I2STX_FIFO_1_RST_N_SOFT,
      I2STX_IF_1_RST_N_SOFT,
      I2SRX_FIFO_0_RST_N_SOFT,
      I2SRX_IF_0_RST_N_SOFT,
      I2SRX_FIFO_1_RST_N_SOFT,
      I2SRX_IF_1_RST_N_SOFT,

      LCD_INTERFACE_PNRES_SOFT = I2SRX_IF_1_RST_N_SOFT + 6,
      SPI_PNRES_APB_SOFT,
      SPI_PNRES_IP_SOFT,
      DMA_PNRES_SOFT,
      NANDFLASH_ECC_RESET_N_SOFT,
      NANDFLASH_AES_RESET_N_SOFT,
      NANDFLASH_NAND_RESET_N_SOFT,
      RNG_RESETN_SOFT,
      SD_MMC_PNRES_SOFT,
      SD_MMC_NRES_CCLK_IN_SOFT,
      USB_OTG_AHB_RST_N_SOFT,
      RED_CTL_RESET_N_SOFT,
      AHB_MPMC_HRESETN_SOFT,
      AHB_MPMC_REFRESH_RESETN_SOFT,
      INTC_RESETN_SOFT
   } CGU_MOD_ID_T;

   /***********************************************************************
   * CGU driver structures
   **********************************************************************/
   /* CGU HPLL config settings structure type */
   typedef struct
   {
      UNS_32 fin_select;
      UNS_32 ndec;
      UNS_32 mdec;
      UNS_32 pdec;
      UNS_32 selr;
      UNS_32 seli;
      UNS_32 selp;
      UNS_32 mode;
      UNS_32 freq; /* in MHz for driver internal data */
   } CGU_HPLL_SETUP_T;

   /* CGU fractional divider settings structure type */
   typedef struct
   {
      UNS_8 stretch; /* Fractional divider stretch enable. */
      UNS_8 n;       /* Fractional divider nominal nominator */
      UNS_16 m;      /* Fractional divider nominal denominator */
   } CGU_FDIV_SETUP_T;

   /* CGU clocks state */
   typedef struct
   {
      UNS_32 clks_0_31;
      UNS_32 clks_32_63;
      UNS_32 clks_64_92;
   } CGU_CLKS_STATE_T;

   /* CGU sub-domain settings structure type */
   typedef struct
   {
      CGU_FDIV_SETUP_T fdiv_cfg;  /* Fractional divider settings */
      UNS_32 clks; /* all clocks belonging to the sub-domain */
   } CGU_SUB_DOMAIN_CFG_T;

   /* CGU domain settings structure type */
   typedef struct
   {
      CGU_DOMAIN_ID_T id;
      UNS_32 fin_sel;
      UNS_32 clk_min;
      UNS_32 clk_cnt;
      UNS_32 fdiv_min;
      UNS_32 fdiv_cnt;
      CGU_SUB_DOMAIN_CFG_T* sub;  /* pointer to array */
   } CGU_DOMAIN_CFG_T;

   /* CGU clks initilisation structure */
   typedef struct
   {
      struct _DOMAIN0_T
      {
         UNS_32 fin_sel;
         CGU_SUB_DOMAIN_CFG_T sub[CGU_SB_BASE0_FDIV_CNT];
      } domain0;

      struct _DOMAIN1_T
      {
         UNS_32 fin_sel;
         CGU_SUB_DOMAIN_CFG_T sub[CGU_SB_BASE1_FDIV_CNT];
      } domain1;

      struct _DOMAIN2_T
      {
         UNS_32 fin_sel;
         CGU_SUB_DOMAIN_CFG_T sub[CGU_SB_BASE2_FDIV_CNT];
      } domain2;

      struct _DOMAIN3_T
      {
         UNS_32 fin_sel;
         CGU_SUB_DOMAIN_CFG_T sub[CGU_SB_BASE3_FDIV_CNT];
      } domain3;

      struct _DOMAIN4_T
      {
         UNS_32 fin_sel;
         CGU_SUB_DOMAIN_CFG_T sub[CGU_SB_BASE4_FDIV_CNT];
      } domain4;

      struct _DOMAIN5_T
      {
         UNS_32 fin_sel;
         CGU_SUB_DOMAIN_CFG_T sub[CGU_SB_BASE5_FDIV_CNT];
      } domain5;

      struct _DOMAIN6_T
      {
         UNS_32 fin_sel;
         CGU_SUB_DOMAIN_CFG_T sub[CGU_SB_BASE6_FDIV_CNT];
      } domain6;

      struct _DOMAIN7_T
      {
         UNS_32 fin_sel;
         CGU_SUB_DOMAIN_CFG_T sub[CGU_SB_BASE7_FDIV_CNT];
      } domain7;

      struct _DOMAIN8_T
      {
         UNS_32 fin_sel;
      } domain8;

      struct _DOMAIN9_T
      {
         UNS_32 fin_sel;
      } domain9;

      struct _DOMAIN10_T
      {
         UNS_32 fin_sel;
         CGU_SUB_DOMAIN_CFG_T sub[CGU_SB_BASE10_FDIV_CNT];
      } domain10;

      struct _DOMAIN11_T
      {
         UNS_32 fin_sel;
      } domain11;

      struct _DYN_FDIV_CFG_T
      {
         UNS_32  sel;
         CGU_FDIV_SETUP_T cfg;
      } dyn_fdiv_cfg[CGU_SB_NR_DYN_FDIV];
   } CGU_CLKS_INIT_T;

   /***********************************************************************
   * CGU driver functions
   **********************************************************************/
   /* Initialize CGU driver */
   void cgu_init(UNS_32 clkin_freq[CGU_FIN_SELECT_MAX]);

   /* Reset all clocks to be sourced from FFAST.  */
   void cgu_reset_all_clks(void);

   /* Initialize all clocks at startup using the defaults structure */
   LPC_STATUS cgu_init_clks(const CGU_CLKS_INIT_T* pClksCfg);

   /* Return the current base frequecy of the requested domain*/
   INT_32 cgu_get_base_freq(CGU_DOMAIN_ID_T baseid);

   /* Change the base frequency for the requested domain */
   void cgu_set_base_freq(CGU_DOMAIN_ID_T baseid, UNS_32 fin_sel);

   /* Return the current frequecy of the requested clock*/
   UNS_32 cgu_get_clk_freq(CGU_CLOCK_ID_T clkid);

   /* Change the sub-domain frequency for the requested clock */
   void cgu_set_subdomain_freq(CGU_CLOCK_ID_T clkid, CGU_FDIV_SETUP_T fdiv_cfg);

   /* Configure the selected HPLL */
   void cgu_hpll_config(CGU_HPLL_ID_T id, CGU_HPLL_SETUP_T* pllsetup);

   /* Get selected HPLL status */
   UNS_32 cgu_hpll_status(CGU_HPLL_ID_T id);

   /* Issue a software reset to the requested module */
   void cgu_soft_reset_module(CGU_MOD_ID_T mod);

   /* enable / disable external enabling of the requested clock in CGU */
   void cgu_clk_set_exten(CGU_CLOCK_ID_T clkid, BOOL_32 enable);

   /* frac divider config function */
   UNS_32 cgu_fdiv_config(UNS_32 fdId,
                          CGU_FDIV_SETUP_T fdivCfg,
                          BOOL_32 enable,
                          BOOL_32 dyn_fdc);

   /***********************************************************************
   * CGU driver inline (ANSI C99 based) functions
   **********************************************************************/
#pragma inline
   static  BOOL_32 cgu_get_watchdog_bark()
   {
      return (CGU_CFG->wd_bark & CGU_WD_BARK);
   }

#pragma inline
   static  BOOL_32 cgu_get_ffast_on()
   {
      return (CGU_CFG->ffast_on & CGU_FFAST_ON);
   }

#pragma inline
   static  void cgu_set_ffast_on(BOOL_32 enable)
   {
      if (enable)
      {
         CGU_CFG->ffast_on = CGU_FFAST_ON;
      }
      else
      {
         CGU_CFG->ffast_on = 0;
      }
   }

#pragma inline
   static  void cgu_set_ffast_bypass(BOOL_32 enable)
   {
      if (enable)
      {
         CGU_CFG->ffast_bypass = CGU_FFAST_BYPASS;
      }
      else
      {
         CGU_CFG->ffast_bypass = 0;
      }
   }

   /* enable / disable the requested clock in CGU */
#pragma inline
   static  void cgu_clk_en_dis(CGU_CLOCK_ID_T clkid, BOOL_32 enable)
   {
      if (enable)
      {
         CGU_SB->clk_pcr[clkid] |= CGU_SB_PCR_RUN;
      }
      else
      {
         CGU_SB->clk_pcr[clkid] &= ~CGU_SB_PCR_RUN;
      }

   }

   /***********************************************************************
   * CGU driver exported global data
   **********************************************************************/
   extern const CGU_CLKS_INIT_T g_cgu_default_clks;
   extern const CGU_CLKS_INIT_T g_lpc314x_cgu_default_clks;

#ifdef __cplusplus
}
#endif

#endif /* LPC313X_CGU_DRIVER_H */
