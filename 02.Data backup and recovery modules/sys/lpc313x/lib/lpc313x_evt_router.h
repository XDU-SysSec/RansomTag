/***********************************************************************
 * $Id:: lpc313x_evt_router.h 1749 2009-03-17 19:44:01Z pdurgesh       $
 *
 * Project: LPC313X Event Router definitions
 *
 * Description:
 *     This file contains the structure definitions and manifest
 *     constants for the LPC313X chip family component:
 *         Event Router
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

#ifndef LPC313X_EVT_ROUTER_H
#define LPC313X_EVT_ROUTER_H

#include "lpc_types.h"
#include "lpc313x_chip.h"

#ifdef __cplusplus
extern "C"
{
#endif

   /***********************************************************************
   * Event Router Register Structures
   **********************************************************************/

   /* Event Router Module Register Structure */
   typedef volatile struct
   {
      // LSB
      volatile const UNS_32 _intReq[256];       // not used
      volatile UNS_32 _target[256];             // not used
      volatile UNS_32 _notused_0[256];          // not used
      volatile const UNS_32 pend[8];            // Pending interrupts read only
      volatile UNS_32 intClr[8];                // clear interrupts write only
      volatile UNS_32 intSet[8];                // set interrupts write only
      volatile UNS_32 mask[8];                  // Mask bits for interrupt inputs R/W
      volatile UNS_32 maskClr[8];               // clear Mask bits write only
      volatile UNS_32 maskSet[8];               // set Mask bits write only
      volatile UNS_32 apr[8];                   // Activation polarity register R/W
      volatile UNS_32 atr[8];                   // Activation type register R/W
      volatile UNS_32 _notused_1[8];            // not used
      volatile const UNS_32 rsr[8];             // Raw status register RO
      volatile UNS_32 intout;                   // interrupt outputs RO
      volatile UNS_32 _notused_2[7];            // unused 0xD40-0xD5C
      volatile UNS_32 _notused_3[40];           // unused 0xD60-0xDFC
      volatile UNS_32 features;                 // Template info RO
      volatile UNS_32 _notused_4[63];           // unused 0xE04-EFC
      volatile UNS_32 _notused_5[63];           // unused 0xF00-FF8
      volatile UNS_32 moduleId;                 // ID and version RO
      volatile const UNS_32 intoutPend[32][8];  // Per output pending status RO
      volatile UNS_32 intoutMask[32][8];        // Per output event mask R/W
      volatile UNS_32 intoutMaskClr[32][8];     // Clear bits in intoutMask WO
      volatile UNS_32 intoutMaskSet[32][8];     // Set bits in intoutMask WO
      // MSB
   } EVENT_ROUTER_REGS_T;

#define EVT_MAX_VALID_BANKS   4
#define EVT_MAX_VALID_INT_OUT 5

   /* Activation polarity register defines */
#define EVT_APR_HIGH    1
#define EVT_APR_LOW     0
#define EVT_APR_BANK0_DEF 0x00000001
#define EVT_APR_BANK1_DEF 0x00000000
#define EVT_APR_BANK2_DEF 0x00000000
#define EVT_APR_BANK3_DEF 0x0FFFFFFC

   /* Activation type register defines */
#define EVT_ATR_EDGE    1
#define EVT_ATR_LEVEL   0
#define EVT_ATR_BANK0_DEF 0x00000001
#define EVT_ATR_BANK1_DEF 0x00000000
#define EVT_ATR_BANK2_DEF 0x00000000
#define EVT_ATR_BANK3_DEF 0x077FFFFC

   /* event PIN or internal signal */
   typedef enum _EVT_TYPE_
   {
      EVT_ipint_int,
      EVT_mLCD_DB_0,
      EVT_mLCD_DB_1,
      EVT_mLCD_DB_2,
      EVT_mLCD_DB_3,
      EVT_mLCD_DB_4,
      EVT_mLCD_DB_5,
      EVT_mLCD_DB_6,
      EVT_mLCD_DB_7,
      EVT_mLCD_DB_8,
      EVT_mLCD_DB_9,
      EVT_mLCD_DB_10,
      EVT_mLCD_DB_11,
      EVT_mLCD_DB_12,
      EVT_mLCD_DB_13,
      EVT_mLCD_DB_14,
      EVT_mLCD_DB_15,
      EVT_mLCD_RS,
      EVT_mLCD_CSB,
      EVT_mLCD_E_RD,
      EVT_mLCD_RW_WR,
      EVT_mNAND_RYBN0,
      EVT_mNAND_RYBN1,
      EVT_mNAND_RYBN2,
      EVT_mNAND_RYBN3,
      EVT_EBI_D_0,
      EVT_EBI_D_1,
      EVT_EBI_D_2,
      EVT_EBI_D_3,
      EVT_EBI_D_4,
      EVT_EBI_D_5,
      EVT_EBI_D_6,
      EVT_EBI_D_7,
      EVT_EBI_D_8,
      EVT_EBI_D_9,
      EVT_EBI_D_10,
      EVT_EBI_D_11,
      EVT_EBI_D_12,
      EVT_EBI_D_13,
      EVT_EBI_D_14,
      EVT_EBI_D_15,
      EVT_EBI_NWE,
      EVT_EBI_A_0_ALE,
      EVT_EBI_A_1_CLE,
      EVT_EBI_DQM_0_NOE,
      EVT_EBI_NCAS_BLOUT_0,
      EVT_EBI_NRAS_BLOUT_1,
      EVT_GPIO1,
      EVT_GPIO0,
      EVT_GPIO2,
      EVT_GPIO3,
      EVT_GPIO4,
      EVT_mGPIO5,
      EVT_mGPIO6,
      EVT_mGPIO7,
      EVT_mGPIO8,
      EVT_mGPIO9,
      EVT_mGPIO10,
      EVT_GPIO11,
      EVT_GPIO12,
      EVT_GPIO13,
      EVT_GPIO14,
      EVT_GPIO15,
      EVT_GPIO16,
      EVT_GPIO17,
      EVT_GPIO18,
      EVT_NAND_NCS_0,
      EVT_NAND_NCS_1,
      EVT_NAND_NCS_2,
      EVT_NAND_NCS_3,
      EVT_SPI_MISO,
      EVT_SPI_MOSI,
      EVT_SPI_CS_IN,
      EVT_SPI_SCK,
      EVT_SPI_CS_OUT0,
      EVT_UART_RXD,
      EVT_UART_TXD,
      EVT_mUART_CTS_N,
      EVT_mUART_RTS_N,
      EVT_mDAO_CLK0,
      EVT_mDAO_BCK0,
      EVT_mDAO_DATA0,
      EVT_mDAO_WS0,
      EVT_DAI_BCK0,
      EVT_DAI_DATA0,
      EVT_DAI_WS0,
      EVT_DAI_DATA1,
      EVT_DAI_BCK1,
      EVT_DAI_WS1,
      EVT_DAO_DATA1,
      EVT_DAO_BCK1,
      EVT_DAO_WS1,
      EVT_CLK_256FS_O,
      EVT_I2C_SDA1,
      EVT_I2C_SCL1,
      EVT_PWM_DATA,
      EVT_AD_NINT_I,
      EVT_PLAY_DET_I,
      EVT_timer0_intct1,
      EVT_timer1_intct1,
      EVT_timer2_intct1,
      EVT_timer3_intct1,
      EVT_adc_int,
      EVT_wdog_m0,
      EVT_uart_rxd,
      EVT_i2c0_scl_n,
      EVT_i2c1_scl_n,
      EVT_arm926_nfiq,
      EVT_arm926_nirq,
      EVT_MCI_DAT_0,
      EVT_MCI_DAT_1,
      EVT_MCI_DAT_2,
      EVT_MCI_DAT_3,
      EVT_MCI_DAT_4,
      EVT_MCI_DAT_5,
      EVT_MCI_DAT_6,
      EVT_MCI_DAT_7,
      EVT_MCI_CMD,
      EVT_MCI_CLK,
      EVT_USB_VBUS1,
      EVT_usb_otg_ahb_needclk,
      EVT_usb_atx_pll_lock,
      EVT_usb_otg_vbus_pwr_en,
      EVT_USB_ID,
      EVT_isram0_mrc_finished,
      EVT_isram1_mrc_finished,
      EVT_LAST
   } EVENT_T;

   /* Event output vector. On this chip we can route an event on 5 vectors. */
   typedef enum _EVT_VEC_OUT_
   {
      EVT_VEC_ROUTER0 = 0,
      EVT_VEC_ROUTER1,
      EVT_VEC_ROUTER2,
      EVT_VEC_ROUTER3,
      EVT_VEC_CGU_WAKEUP
   } EVT_VEC_OUT_T;

   /* Macros to compute the bank based on EVENT_T */
#define EVT_GET_BANK(evt)   (((evt) >> 5) & 0x3)

   /* Macro pointing to SysRegs registers */
#define EVTR_REGS  ((EVENT_ROUTER_REGS_T  *)(EVENT_ROUTER_BASE))

#ifdef __cplusplus
}
#endif


#endif /* LPC313X_EVT_ROUTER_H */
