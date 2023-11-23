/***********************************************************************
 * $Id:: lpc313x_cgu.h 1529 2009-01-06 02:08:40Z pdurgesh              $
 *
 * Project: LPC313X family chip definitions
 *
 * Description:
 *     This file contains the structure definitions and manifest
 *     constants for the LPC313x chip family component:
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

#ifndef LPC313X_CGU_H
#define LPC313X_CGU_H

/**********************************************************************
* Include files
**********************************************************************/

#include "lpc_types.h"
#include "lpc313x_chip.h"

#ifdef __cplusplus
extern "C"    /* Assume C declarations for C++ */
{
#endif

   /**********************************************************************
   * The device registers
   **********************************************************************/
   /* ----------------
   * HP PLL Registers
   * ----------------
   */
   typedef volatile struct
   {
      volatile UNS_32 fin_select;
      volatile UNS_32 mdec;
      volatile UNS_32 ndec;
      volatile UNS_32 pdec;
      volatile UNS_32 mode;
      volatile UNS_32 status;
      volatile UNS_32 ack;
      volatile UNS_32 req;
      volatile UNS_32 inselr;
      volatile UNS_32 inseli;
      volatile UNS_32 inselp;
      volatile UNS_32 selr;
      volatile UNS_32 seli;
      volatile UNS_32 selp;
   } CGU_HP_CFG_REGS, *pCGU_HP_CFG_REGS;


   typedef volatile struct
   {
      volatile UNS_32 powermode;
      volatile UNS_32 wd_bark;
      volatile UNS_32 ffast_on;
      volatile UNS_32 ffast_bypass;
      volatile UNS_32 apb0_resetn_soft;
      volatile UNS_32 ahb2apb0_pnres_soft;
      volatile UNS_32 apb1_resetn_soft;
      volatile UNS_32 ahb2apb1_pnres_soft;
      volatile UNS_32 apb2_resetn_soft;
      volatile UNS_32 ahb2apb2_pnres_soft;
      volatile UNS_32 apb3_resetn_soft;
      volatile UNS_32 ahb2apb3_pnres_soft;
      volatile UNS_32 apb4_resetn_soft;
      volatile UNS_32 ahb2intc_resetn_soft;
      volatile UNS_32 ahb0_resetn_soft;
      volatile UNS_32 ebi_resetn_soft;
      volatile UNS_32 pcm_pnres_soft;
      volatile UNS_32 pcm_reset_n_soft;
      volatile UNS_32 pcm_reset_async_n_soft;
      volatile UNS_32 timer0_pnres_soft;
      volatile UNS_32 timer1_pnres_soft;
      volatile UNS_32 timer2_pnres_soft;
      volatile UNS_32 timer3_pnres_soft;
      volatile UNS_32 adc_presetn_soft;
      volatile UNS_32 adc_resetn_adc10bits_soft;
      volatile UNS_32 pwm_reset_an_soft;
      volatile UNS_32 uart_sys_rst_an_soft;
      volatile UNS_32 i2c0_pnres_soft;
      volatile UNS_32 i2c1_pnres_soft;
      volatile UNS_32 i2s_cfg_rst_n_soft;
      volatile UNS_32 i2s_nsof_rst_n_soft;
      volatile UNS_32 edge_det_rst_n_soft;
      volatile UNS_32 i2stx_fifo_0_rst_n_soft;
      volatile UNS_32 i2stx_if_0_rst_n_soft;
      volatile UNS_32 i2stx_fifo_1_rst_n_soft;
      volatile UNS_32 i2stx_if_1_rst_n_soft;
      volatile UNS_32 i2srx_fifo_0_rst_n_soft;
      volatile UNS_32 i2srx_if_0_rst_n_soft;
      volatile UNS_32 i2srx_fifo_1_rst_n_soft;
      volatile UNS_32 i2srx_if_1_rst_n_soft;
      volatile UNS_32 rserved[5];
      volatile UNS_32 lcd_interface_pnres_soft;
      volatile UNS_32 spi_pnres_apb_soft;
      volatile UNS_32 spi_pnres_ip_soft;
      volatile UNS_32 dma_pnres_soft;
      volatile UNS_32 nandflash_ctrl_ecc_reset_n_soft;
      volatile UNS_32 nandflash_ctrl_aes_reset_n_soft;
      volatile UNS_32 nandflash_ctrl_nand_reset_n_soft;
      volatile UNS_32 rng_resetn_soft;
      volatile UNS_32 sd_mmc_pnres_soft;
      volatile UNS_32 sd_mmc_nres_cclk_in_soft;
      volatile UNS_32 usb_otg_ahb_rst_n_soft;
      volatile UNS_32 red_ctl_reset_n_soft;
      volatile UNS_32 ahb_mpmc_hresetn_soft;
      volatile UNS_32 ahb_mpmc_refresh_resetn_soft;
      volatile UNS_32 intc_resetn_soft;
      CGU_HP_CFG_REGS hp[2];
   } CGU_CONFIG_REGS, *pCGU_CONFIG_REGS;



   /**********************************************************************
   * Register description of POWERMODE
   **********************************************************************/
#define CGU_POWERMODE_MASK     0x3
#define CGU_POWERMODE_NORMAL   0x1
#define CGU_POWERMODE_WAKEUP   0x3

   /**********************************************************************
   * Register description of WD_BARK
   **********************************************************************/
#define CGU_WD_BARK            0x1

   /**********************************************************************
   * Register description of FFAST_ON
   **********************************************************************/
#define CGU_FFAST_ON           0x1

   /**********************************************************************
   * Register description of FFAST_BYPASS
   **********************************************************************/
#define CGU_FFAST_BYPASS       0x1

   /**********************************************************************
   * Register description of soft reset registers
   **********************************************************************/
#define CGU_CONFIG_SOFT_RESET  0x1

   /**********************************************************************
   * Register description of HPll REGISTERS
   **********************************************************************/
//#define CGU_HPLL0_ID      0
//#define CGU_HPLL1_ID      1

   /**********************************************************************
   * Register description of HP_FIN_SELECT
   **********************************************************************/
#define CGU_HPLL_FIN_SEL_MASK       0xf
#define CGU_FIN_SELECT_FFAST        0x0
#define CGU_FIN_SELECT_XT_I2SRX_BCK0  0x1
#define CGU_FIN_SELECT_XT_I2SRX_WS0   0x2
#define CGU_FIN_SELECT_XT_I2SRX_BCK1  0x3
#define CGU_FIN_SELECT_XT_I2SRX_WS1   0x4
#define CGU_FIN_SELECT_HPPLL0       0x5
#define CGU_FIN_SELECT_HPPLL1       0x6
#define CGU_FIN_SELECT_MAX          7

   /**********************************************************************
   * Register description of HP_MDEC
   **********************************************************************/
#define CGU_HPLL_MDEC_MASK          0x1ffff
   /**********************************************************************
   * Register description of HP_NDEC
   **********************************************************************/
#define CGU_HPLL_NDEC_MSK           0x3ff
   /**********************************************************************
   * Register description of HP_PDEC
   **********************************************************************/
#define CGU_HPLL_PDEC_MSK           0x7f
   /**********************************************************************
   * Register description of HP_MODE
   **********************************************************************/
#define CGU_HPLL_MODE_POR_VAL       0x6
#define CGU_HPLL_MODE_CLKEN         _BIT(0)
#define CGU_HPLL_MODE_SKEWEN        _BIT(1)
#define CGU_HPLL_MODE_PD            _BIT(2)
#define CGU_HPLL_MODE_DIRECTO       _BIT(3)
#define CGU_HPLL_MODE_DIRECTI       _BIT(4)
#define CGU_HPLL_MODE_FRM           _BIT(5)
#define CGU_HPLL_MODE_BANDSEL       _BIT(6)
#define CGU_HPLL_MODE_LIMUP_OFF     _BIT(7)
#define CGU_HPLL_MODE_BYPASS        _BIT(8)

   /**********************************************************************
   * Register description of HP1_STATUS
   **********************************************************************/
#define CGU_HPLL_STATUS_FR          _BIT(1)
#define CGU_HPLL_STATUS_LOCK        _BIT(0)

   /**********************************************************************
   * Register description of HP_ACK & HP_REQ
   **********************************************************************/
#define CGU_HPLL_ACK_P              _BIT(2)
#define CGU_HPLL_ACK_N              _BIT(1)
#define CGU_HPLL_ACK_M              _BIT(0)

   /**********************************************************************
   * Register description of HP1_INSELR
   **********************************************************************/
#define CGU_HPLL_INSELR_MASK        0xf
   /**********************************************************************
   * Register description of HP1_INSELI
   **********************************************************************/
#define CGU_HPLL_INSELI_MASK        0x3f
   /**********************************************************************
   * Register description of HP1_INSELP
   **********************************************************************/
#define CGU_HPLL_INSELP_MASK        0x1f
   /**********************************************************************
   * Register description of HP1_SELR
   **********************************************************************/
#define CGU_HPLL_SELR_MASK          0xf
   /**********************************************************************
   * Register description of HP1_SELI
   **********************************************************************/
#define CGU_HPLL_SELI_MASK          0x3f
   /**********************************************************************
   * Register description of HP1_SELP
   **********************************************************************/
#define CGU_HPLL_SELP_MASK          0x1f

   /* Macro pointing to CGU configuration registers */
#define CGU_CFG  ((CGU_CONFIG_REGS  *)(CGU_BASE))

#ifdef __cplusplus
}
#endif

#endif /* LPC313X_CGU_H */

