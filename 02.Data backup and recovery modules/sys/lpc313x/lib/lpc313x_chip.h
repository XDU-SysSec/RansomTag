/***********************************************************************
 * $Id:: lpc313x_chip.h 2554 2009-11-24 23:06:41Z pdurgesh             $
 *
 * Project: LPC313X family chip definitions
 *
 * Description:
 *     This file contains chip specific information such as the
 *     physical addresses defines for the LPC313X registers, clock
 *     frequencies, and other chip information.
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
 
#ifndef LPC313X_CHIP_H
#define LPC313X_CHIP_H

#ifdef __cplusplus
extern "C"    /* Assume C declarations for C++ */
{
#endif


   /* --------------------------------------------------------------------------
   *  AHB0_SLAVE_PORT9 bus
   *  -------------------------------------------------------------------------- */
#define ISRAM_BASE               0x11028000
#define ISRAM_LENGTH             0x00030000

   /* --------------------------------------------------------------------------
   *  isram subbase bus
   *  -------------------------------------------------------------------------- */
#define ISRAM_ESRAM0_BASE        0x11028000
#define ISRAM_ESRAM0_LENGTH      0x00018000
#define ISRAM_ESRAM1_BASE        0x11040000
#define ISRAM_ESRAM1_LENGTH      0x00018000


   /* --------------------------------------------------------------------------
   *  AHB0_SLAVE_PORT11 bus
   *  -------------------------------------------------------------------------- */
#define ISROM_BASE                0x12000000
#define ISROM_LENGTH              0x00020000
#define ISROM_MMU_TTB_BASE        0x1201C000
#define ISROM_CRC32_TABLE         0x12015CBC

   /* --------------------------------------------------------------------------
   *  AHB0_SLAVE_PORT0 bus
   *  -------------------------------------------------------------------------- */
#define APB0_BASE                 0x13000000
#define APB0_LENGTH               0x00008000

   /* --------------------------------------------------------------------------
   *  APB0 bus
   *  -------------------------------------------------------------------------- */
#define EVENT_ROUTER_BASE         0x13000000
#define EVENT_ROUTER_LENGTH       0x00002000
#define ADC_BASE                  0x13002000
#define ADC_LENGTH                0x00000400
#define WDOG_BASE                 0x13002400
#define WDOG_LENGTH               0x00000400
#define SYSCREG_BASE              0x13002800
#define SYSCREG_LENGTH            0x00000400
#define IOCONF_BASE               0x13003000
#define IOCONF_LENGTH             0x00001000
#define CGU_SWITCHBOX_BASE        0x13004000
#define CGU_SWITCHBOX_LENGTH      0x00000c00
#define CGU_BASE                  0x13004c00
#define CGU_LENGTH                0x00000400


   /* --------------------------------------------------------------------------
   *  APB0 bus
   *  -------------------------------------------------------------------------- */
#define OTP_BASE                  0x13005000
#define OTP_LENGTH                0x00001000
#define RNG_BASE                  0x13006000
#define RNG_LENGTH                0x00001000

   /* --------------------------------------------------------------------------
   *  Chip ID register defines
   *  -------------------------------------------------------------------------- */
#define CHIP_ID_REG               0x13005048
#define CHIP_ID_MSK               0x0000003F
#define CHIP_ID_LPC3141           0x0E
#define CHIP_ID_LPC3143           0x0B
#define CHIP_ID_LPC3152           0x0D
#define CHIP_ID_LPC3154           0x07

   /* --------------------------------------------------------------------------
   *  AHB0_SLAVE_PORT1 bus
   *  -------------------------------------------------------------------------- */
#define APB1_BASE                 0x13008000
#define APB1_LENGTH               0x00004000

   /* --------------------------------------------------------------------------
   *  APB1 bus
   *  -------------------------------------------------------------------------- */
#define TIMER0_BASE               0x13008000
#define TIMER0_LENGTH             0x00000400
#define TIMER1_BASE               0x13008400
#define TIMER1_LENGTH             0x00000400
#define TIMER2_BASE               0x13008800
#define TIMER2_LENGTH             0x00000400
#define TIMER3_BASE               0x13008c00
#define TIMER3_LENGTH             0x00000400
#define PWM_BASE                  0x13009000
#define PWM_LENGTH                0x00001000

   /* --------------------------------------------------------------------------
   *  pwm subbase bus
   *  -------------------------------------------------------------------------- */
#define PWM_0_BASE                0x13009000
#define PWM_0_LENGTH              0x00000008

   /* --------------------------------------------------------------------------
   *  APB1 bus
   *  -------------------------------------------------------------------------- */
#define I2C0_BASE                 0x1300a000
#define I2C0_LENGTH               0x00000400
#define I2C1_BASE                 0x1300a400
#define I2C1_LENGTH               0x00000400

   /* --------------------------------------------------------------------------
   *  AHB0_SLAVE_PORT2 bus
   *  -------------------------------------------------------------------------- */
#define APB2_BASE                 0x15000000
#define APB2_LENGTH               0x00004000

   /* --------------------------------------------------------------------------
   *  APB2 bus
   *  -------------------------------------------------------------------------- */
#define IPINT_BASE                0x15000000
#define IPINT_LENGTH              0x00000400
#define LCD_INTERFACE_BASE        0x15000400
#define LCD_INTERFACE_LENGTH      0x00000400
#define UART_BASE                 0x15001000
#define UART_LENGTH               0x00001000
#define SPI_BASE                  0x15002000
#define SPI_LENGTH                0x00001000

   /* --------------------------------------------------------------------------
   *  AHB0_SLAVE_PORT3 bus
   *  -------------------------------------------------------------------------- */
#define APB3_BASE                 0x16000000
#define APB3_LENGTH               0x00000400

   /* --------------------------------------------------------------------------
   *  adss_cfg subbase bus
   *  -------------------------------------------------------------------------- */
#define ADSS_CFG_BASE             0x16000000
#define ADSS_CFG_LENGTH           0x00000080
#define I2STX0_BASE               0x16000080
#define I2STX0_LENGTH             0x00000080
#define I2STX1_BASE               0x16000100
#define I2STX1_LENGTH             0x00000080
#define I2SRX0_BASE               0x16000180
#define I2SRX0_LENGTH             0x00000080
#define I2SRX1_BASE               0x16000200
#define I2SRX1_LENGTH             0x00000080

   /* --------------------------------------------------------------------------
   *  AHB0_SLAVE_PORT4 bus
   *  -------------------------------------------------------------------------- */
#define APB4_BASE                 0x17000000
#define APB4_LENGTH               0x00001000

   /* --------------------------------------------------------------------------
   *  APB4 bus
   *  -------------------------------------------------------------------------- */
#define DMA_BASE                  0x17000000
#define DMA_LENGTH                0x00000800
#define NANDFLASH_CTRL_CFG_BASE   0x17000800
#define NANDFLASH_CTRL_CFG_LENGTH 0x00000800

   /* --------------------------------------------------------------------------
   *  AHB0_SLAVE_PORT12 bus
   *  -------------------------------------------------------------------------- */
#define MPMC_CFG_BASE             0x17008000
#define MPMC_CFG_LENGTH           0x00001000

   /* --------------------------------------------------------------------------
   *  AHB0_SLAVE_PORT7 bus
   *  -------------------------------------------------------------------------- */
#define SD_MMC_BASE               0x18000000
#define SD_MMC_LENGTH             0x00000400

   /* --------------------------------------------------------------------------
   *  AHB0_SLAVE_PORT8 bus
   *  -------------------------------------------------------------------------- */
#define USBOTG_BASE               0x19000000
#define USBOTG_LENGTH             0x00001000

   /* --------------------------------------------------------------------------
   *  ahb_mpmc_pl172_s0 subbase bus
   *  -------------------------------------------------------------------------- */
#define EXT_SRAM0_0_BASE          0x20000000
#define EXT_SRAM0_0_LENGTH        0x00010000
#define EXT_SRAM0_1_BASE          0x20020000
#define EXT_SRAM0_1_LENGTH        0x00010000
#define EXT_SDRAM_BASE            0x30000000
#define EXT_SDRAM_LENGTH          0x02000000

   /* --------------------------------------------------------------------------
   *  AHB0_SLAVE_PORT5 bus
   *  -------------------------------------------------------------------------- */
#define AHB2MMIO_BASE             0x60000000
#define AHB2MMIO_LENGTH           0x00001000

   /* --------------------------------------------------------------------------
   *  ahb2mmio bus
   *  -------------------------------------------------------------------------- */
#define INTC_BASE                 0x60000000
#define INTC_LENGTH               0x00001000

   /* --------------------------------------------------------------------------
   *  AHB0_SLAVE_PORT6 bus
   *  -------------------------------------------------------------------------- */
#define NANDFLASH_CTRL_S0_BASE    0x70000000
#define NANDFLASH_CTRL_S0_LENGTH  0x00000800

#ifdef __cplusplus
}
#endif

#endif /*LPC313X_CHIP_H*/
