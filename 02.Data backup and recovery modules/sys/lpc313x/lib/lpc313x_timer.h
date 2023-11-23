/***********************************************************************
 * $Id:: lpc313x_timer.h 1180 2008-09-11 19:32:40Z pdurgesh            $
 *
 * Project: LPC313X Timer Module definitions
 *
 * Description:
 *     This file contains the structure definitions and manifest
 *     constants for the LPC313X chip family component:
 *         Timer Module
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

#ifndef LPC313X_TIMER_H
#define LPC313X_TIMER_H

#include "lpc_types.h"
#include "lpc313x_chip.h"

#ifdef __cplusplus
extern "C"
{
#endif

   /***********************************************************************
   * Timer Module Register Structures
   **********************************************************************/

   /* Timer Module Register Structure */
   typedef volatile struct
   {
      /* LSB */
      volatile UNS_32 load;      /* Timer load               */
      volatile const UNS_32 value;/* Timer Counter          */
      volatile UNS_32 control;    /* Timer Control Register */
      volatile UNS_32 clear;      /* clears the interrupt      */
      volatile UNS_32 test;       /* validation purpose       */
      /* MSB */
   } TIMER_REGS_T;

   /**********************************************************************
   *  Timer Control Register (TimerCtrl) (0x08) Read/Write
   **********************************************************************/
#define TM_CTRL_ENABLE    _BIT(7)
#define TM_CTRL_MODE      _BIT(6)
#define TM_CTRL_PERIODIC  _BIT(6)
#define TM_CTRL_PS1       _SBF(2, 0)
#define TM_CTRL_PS16      _SBF(2, 1)
#define TM_CTRL_PS256     _SBF(2, 2)
#define TM_CTRL_PS_MASK   _SBF(2, 0x3)

   /**********************************************************************
   * Macro to access TIMER registers
   **********************************************************************/
#define TIMER_CNTR0       ((TIMER_REGS_T*)TIMER0_BASE)
#define TIMER_CNTR1       ((TIMER_REGS_T*)TIMER1_BASE)
#define TIMER_CNTR2       ((TIMER_REGS_T*)TIMER2_BASE)
#define TIMER_CNTR3       ((TIMER_REGS_T*)TIMER3_BASE)



#ifdef __cplusplus
}
#endif

#endif /* LPC313X_TIMER_H */
