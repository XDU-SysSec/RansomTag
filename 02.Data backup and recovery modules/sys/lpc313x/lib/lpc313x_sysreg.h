/***********************************************************************
 * $Id:: lpc313x_sysreg.h 1180 2008-09-11 19:32:40Z pdurgesh           $
 *
 * Project: LPC313X System Control Registers (SysCReg) definitions
 *
 * Description:
 *     This file contains the structure definitions and manifest
 *     constants for the LPC313X chip family component:
 *         System Control Registers (SysCReg)
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

#ifndef LPC313X_SYSREG_H
#define LPC313X_SYSREG_H

#include "lpc_types.h"
#include "lpc313x_chip.h"

#ifdef __cplusplus
extern "C"
{
#endif

   /***********************************************************************
   * System Control (SysCReg) Register Structures
   **********************************************************************/
   typedef volatile struct
   {
      volatile UNS_32 spare_reg0;
      volatile UNS_32 activate_testpins;
      volatile UNS_32 ebi_ip2024_1;
      volatile UNS_32 ebi_ip2024_2;
      volatile UNS_32 ebi_ip2024_3;
      volatile UNS_32 ccp_ring_osc_cfg;
      volatile UNS_32 ssa1_adc_pd_adc10bits;
      volatile UNS_32 cgu_dyn_hp0;
      volatile UNS_32 cgu_dyn_hp1;
      volatile UNS_32 abc_cfg;
      volatile UNS_32 sd_mmc_cfg;
      volatile UNS_32 mci_delaymodes;
      volatile UNS_32 usb_atx_pll_pd_reg;
      volatile UNS_32 usb_otg_cfg;
      volatile UNS_32 usb_otg_port_ind_ctl;
      volatile UNS_32 sys_usb_tpr_dyn;
      volatile UNS_32 usb_pll_ndec;
      volatile UNS_32 usb_pll_mdec;
      volatile UNS_32 usb_pll_pdec;
      volatile UNS_32 usb_pll_selr;
      volatile UNS_32 usb_pll_seli;
      volatile UNS_32 usb_pll_selp;
      volatile UNS_32 isram0_latency_cfg;
      volatile UNS_32 isram1_latency_cfg;
      volatile UNS_32 isrom_latency_cfg;
      volatile UNS_32 ahb_mpmc_pl172_misc;
      volatile UNS_32 mpmp_delaymodes;
      volatile UNS_32 mpmc_waitread_delay0;
      volatile UNS_32 mpmc_waitread_delay1;
      volatile UNS_32 wire_ebi_msize_init;
      volatile UNS_32 mpmc_testmode0;
      volatile UNS_32 mpmc_testmode1;
      volatile UNS_32 ahb0_extprio;
      volatile UNS_32 arm926_shadow_pointer;
      volatile UNS_32 sleepstatus;
      volatile UNS_32 chip_id;
      volatile UNS_32 mux_lcd_ebi_sel;
      volatile UNS_32 mux_gpio_mci_sel;
      volatile UNS_32 mux_nand_mci_sel;
      volatile UNS_32 mux_uart_spi_sel;
      volatile UNS_32 mux_dao_ipint_sel;
      volatile UNS_32 ebi_d_9_pctrl;
      volatile UNS_32 ebi_d_10_pctrl;
      volatile UNS_32 ebi_d_11_pctrl;
      volatile UNS_32 ebi_d_12_pctrl;
      volatile UNS_32 ebi_d_13_pctrl;
      volatile UNS_32 ebi_d_14_pctrl;
      volatile UNS_32 dai_bck0_pctrl;
      volatile UNS_32 mgpio9_pctrl;
      volatile UNS_32 mgpio6_pctrl;
      volatile UNS_32 mlcd_db_7_pctrl;
      volatile UNS_32 mlcd_db_4_pctrl;
      volatile UNS_32 mlcd_db_2_pctrl;
      volatile UNS_32 mnand_rybn0_pctrl;
      volatile UNS_32 gpio1_pctrl;
      volatile UNS_32 ebi_d_4_pctrl;
      volatile UNS_32 mdao_clk0_pctrl;
      volatile UNS_32 mdao_bck0_pctrl;
      volatile UNS_32 ebi_a_1_cle_pctrl;
      volatile UNS_32 ebi_ncas_blout_0_pctrl;
      volatile UNS_32 nand_ncs_3_pctrl;
      volatile UNS_32 mlcd_db_0_pctrl;
      volatile UNS_32 ebi_dqm_0_noe_pctrl;
      volatile UNS_32 ebi_d_0_pctrl;
      volatile UNS_32 ebi_d_1_pctrl;
      volatile UNS_32 ebi_d_2_pctrl;
      volatile UNS_32 ebi_d_3_pctrl;
      volatile UNS_32 ebi_d_5_pctrl;
      volatile UNS_32 ebi_d_6_pctrl;
      volatile UNS_32 ebi_d_7_pctrl;
      volatile UNS_32 ebi_d_8_pctrl;
      volatile UNS_32 ebi_d_15_pctrl;
      volatile UNS_32 dao_data1_pctrl;
      volatile UNS_32 dao_bck1_pctrl;
      volatile UNS_32 dao_ws1_pctrl;
      volatile UNS_32 dai_data0_pctrl;
      volatile UNS_32 dai_ws0_pctrl;
      volatile UNS_32 dai_data1_pctrl;
      volatile UNS_32 dai_bck1_pctrl;
      volatile UNS_32 dai_ws1_pctrl;
      volatile UNS_32 sysclk_o_pctrl;
      volatile UNS_32 pwm_data_pctrl;
      volatile UNS_32 uart_rxd_pctrl;
      volatile UNS_32 uart_txd_pctrl;
      volatile UNS_32 i2c_sda1_pctrl;
      volatile UNS_32 i2c_scl1_pctrl;
      volatile UNS_32 clk_256fs_o_pctrl;
      volatile UNS_32 gpio0_pctrl;
      volatile UNS_32 gpio2_pctrl;
      volatile UNS_32 gpio3_pctrl;
      volatile UNS_32 gpio4_pctrl;
      volatile UNS_32 gpio_tst_0_dd_pctrl;
      volatile UNS_32 gpio_tst_1_dd_pctrl;
      volatile UNS_32 gpio_tst_2_dd_pctrl;
      volatile UNS_32 gpio_tst_3_dd_pctrl;
      volatile UNS_32 gpio_tst_4_dd_pctrl;
      volatile UNS_32 gpio_tst_5_dd_pctrl;
      volatile UNS_32 gpio_tst_6_dd_pctrl;
      volatile UNS_32 gpio_tst_7_dd_pctrl;
      volatile UNS_32 ad_nint_i_pctrl;
      volatile UNS_32 play_det_i_pctrl;
      volatile UNS_32 spi_miso_pctrl;
      volatile UNS_32 spi_mosi_pctrl;
      volatile UNS_32 spi_cs_in_pctrl;
      volatile UNS_32 spi_sck_pctrl;
      volatile UNS_32 spi_cs_out0_pctrl;
      volatile UNS_32 nand_ncs_0_pctrl;
      volatile UNS_32 nand_ncs_1_pctrl;
      volatile UNS_32 nand_ncs_2_pctrl;
      volatile UNS_32 mlcd_csb_pctrl;
      volatile UNS_32 mlcd_db_1_pctrl;
      volatile UNS_32 mlcd_e_rd_pctrl;
      volatile UNS_32 mlcd_rs_pctrl;
      volatile UNS_32 mlcd_rw_wr_pctrl;
      volatile UNS_32 mlcd_db_3_pctrl;
      volatile UNS_32 mlcd_db_5_pctrl;
      volatile UNS_32 mlcd_db_6_pctrl;
      volatile UNS_32 mlcd_db_8_pctrl;
      volatile UNS_32 mlcd_db_9_pctrl;
      volatile UNS_32 mlcd_db_10_pctrl;
      volatile UNS_32 mlcd_db_11_pctrl;
      volatile UNS_32 mlcd_db_12_pctrl;
      volatile UNS_32 mlcd_db_13_pctrl;
      volatile UNS_32 mlcd_db_14_pctrl;
      volatile UNS_32 mlcd_db_15_pctrl;
      volatile UNS_32 mgpio5_pctrl;
      volatile UNS_32 mgpio7_pctrl;
      volatile UNS_32 mgpio8_pctrl;
      volatile UNS_32 mgpio10_pctrl;
      volatile UNS_32 mnand_rybn1_pctrl;
      volatile UNS_32 mnand_rybn2_pctrl;
      volatile UNS_32 mnand_rybn3_pctrl;
      volatile UNS_32 muart_cts_n_pctrl;
      volatile UNS_32 muart_rts_n_pctrl;
      volatile UNS_32 mdao_data0_pctrl;
      volatile UNS_32 mdao_ws0_pctrl;
      volatile UNS_32 ebi_nras_blout_1_pctrl;
      volatile UNS_32 ebi_a_0_ale_pctrl;
      volatile UNS_32 ebi_nwe_pctrl;
      volatile UNS_32 eshctrl_sup4;
      volatile UNS_32 eshctrl_sup8;
   } SYSCREG_REGS_T;

   /***********************************************************************
    * SYSREGS Pad control register definitions
    **********************************************************************/
#define SYSREG_PCTRL_PULLUP     _SBF(1, 0x00)
#define SYSREG_PCTRL_PULLDOWN   _SBF(1, 0x03)
#define SYSREG_PCTRL_RPTR       _SBF(1, 0x01)
#define SYSREG_PCTRL_INPUT      _SBF(1, 0x02)

   /* Macro pointing to SysRegs registers */
#define SYS_REGS  ((SYSCREG_REGS_T  *)(SYSCREG_BASE))

#ifdef __cplusplus
}
#endif

#endif /* LPC313X_SYSREG_H */
