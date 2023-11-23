/***********************************************************************
 * $Id:: lpc_arm_arch.h 745 2008-05-13 19:59:29Z pdurgesh              $
 *
 * Project: General Utilities
 *
 * Description:
 *      This file contains constant and macro definitions specific
 *      to the ARM architecture.
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

#ifndef LPC_ARM_ARCH_H
#define LPC_ARM_ARCH_H

/***********************************************************************
 * ARM Hard Vector Address Locations
 **********************************************************************/

#define ARM_RESET_VEC   0x00 /* ARM reset vector address */
#define ARM_UNDEF_VEC   0x04 /* ARM undefined vector address */
#define ARM_SWI_VEC     0x08 /* ARM software interrupt vector address */
#define ARM_IABORT_VEC  0x0C /* ARM prefetch abort vector address */
#define ARM_DABORT_VEC  0x10 /* ARM data abort vector address */
#define ARM_RSVD_VEC    0x14 /* Reserved */
#define ARM_IRQ_VEC     0x18 /* ARM IRQ vector address */
#define ARM_FIQ_VEC     0x1C /* ARM FIQ vector address */

/***********************************************************************
 * ARM Current and Saved Processor Status Register Bits (xPSR)
 **********************************************************************/
/* ARM Condition Code Flag Bits (xPSR bits [31:27]) */
#define ARM_CCFLG_N     0x80000000 /* ARM negative flag */
#define ARM_CCFLG_Z     0x40000000 /* ARM zero flag */
#define ARM_CCFLG_C     0x20000000 /* ARM carry flag */
#define ARM_CCFLG_V     0x10000000 /* ARM overflow flag */
#define ARM_CCFLG_Q     0x08000000 /* ARM 'Q' flag */

/* ARM Interrupt Disable Bits (xPSR bits [7:6]) */
#define ARM_IRQ         0x80 /* ARM IRQ mode */
#define ARM_FIQ         0x40 /* ARM FIQ mode */

/* ARM Thumb State Bit (xPSR bit [5]) */
#define ARM_THUMB       0x20 /* ARM thumb mode */

/* ARM Processor Mode Values (xPSR bits [4:0])
 * Use ARM_MODE macro and constants to test ARM mode
 * Example, where tmp has xPSR value:
 * if (ARM_MODE(tmp, ARM_MODE_IRQ))
 *     statement; */

/* ARM mode selection macro */
#define ARM_MODE(m,n)   (((m) & _BITMASK(5)) == (n))
#define ARM_MODE_USR    0x10 /* ARM user mode */
#define ARM_MODE_FIQ    0x11 /* ARM FIQ mode */
#define ARM_MODE_IRQ    0x12 /* ARM IRQ mode */
#define ARM_MODE_SVC    0x13 /* ARM service mode */
#define ARM_MODE_ABT    0x17 /* ARM abort mode */
#define ARM_MODE_UND    0x1B /* ARM undefined mode */
#define ARM_MODE_SYS    0x1F /* ARM system mode */

#endif /* LPC_ARM_ARCH_H */
