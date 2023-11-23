/***********************************************************************
 * $Id:: lpc313x_timer_driver.h 1201 2008-10-03 19:25:38Z pdurgesh     $
 *
 * Project: LPC313X timer driver
 *
 * Description:
 *     This file contains driver support for the LPC313X timer.
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
 *********************************************************************/

#ifndef LPC313X_TIMER_DRIVER_H
#define LPC313X_TIMER_DRIVER_H

#include "lpc313x_timer.h"

#ifdef __cplusplus
extern "C"
{
#endif

   /***********************************************************************
    * Timer device configuration commands (IOCTL commands and arguments)
    **********************************************************************/

   /* Timer device commands (IOCTL commands) */
   typedef enum
   {
      TMR_ENABLE,      /* Enable or disable the timer, use arg with a
              value of '1' or '0' in arg to enable or
              disable */
      TMR_RESET,       /* Resets the timer count and prescale values, arg
              value does not matter */
      /* Clear a pending interrupt */
      TMR_CLEAR_INTS,
      /* Configure timer to the specified mode & duration*/
      TMR_SET_CFG,
      /* Set a load count value, use arg as load value */
      TMR_SET_LOAD,
      /* Configure timer/counter to periodic mode, use arg as load
      value*/
      TMR_SET_PERIODIC_MODE,
      /* Configure timer/counter to free running mode, use arg as load
      value*/
      TMR_SET_FREERUN_MODE,
      /* Set a prescale divider. Valid arg values are TM_CTRL_PS1 or
      TM_CTRL_PS16 or TM_CTRL_PS256*/
      TMR_SET_PSCALE,
      TMR_SET_MSECS,   /* Set the prescale register count to initiate a
              terminal count increment in the number of
            milliseconds specified in arg */
      TMR_SET_USECS,   /* Set the prescale register count to initiate a
              terminal count increment in the number of
            microseconds specified in arg */
      TMR_GET_STATUS  /* Get a timer status, use a pointer to the
             MST_ARG_T structure with the timer enumeration
             and a arg value of MST_IOCTL_STS_T */
   }
   TMR_IOCTL_CMD_T;

   /* Timer device arguments for TMR_GET_STATUS command (IOCTL
      arguments) */
   typedef enum
   {
      TMR_GET_COUNT,   /* Returns the current timer count */
      TMR_GET_PS_SCALE, /* Returns the current prescaler count */
      TMR_GET_CLOCK  /* Returns the clock rate (Hz) driving the clock */
   } MST_ARG_T;

   /* Structure used for some IOCTLS */
   typedef struct
   {
      INT_32 arg1;
      UNS_32 arg2;
   } TMR_ARG_T;

   /* Structure used for TMR_SET_CFG */
   typedef struct
   {
      BOOL_32 periodic;
      UNS_32 usecs;
      BOOL_32 enable;
   } TMR_CFG_ARG_T;

   /***********************************************************************
    * TIMER driver API functions
    **********************************************************************/

   /* Open the timer */
   INT_32 timer_open(void *ipbase,
                     INT_32 arg);

   /* Close the timer */
   LPC_STATUS timer_close(INT_32 devid);

   /* Timer configuration block */
   LPC_STATUS timer_ioctl(INT_32 devid,
                          INT_32 cmd,
                          INT_32 arg);

   /* Timer read function (stub only) */
   INT_32 timer_read(INT_32 devid,
                     void *buffer,
                     INT_32 max_bytes);

   /* Timer write function (stub only) */
   INT_32 timer_write(INT_32 devid,
                      void *buffer,
                      INT_32 n_bytes);

   /***********************************************************************
    * Other TIMER driver functions
    **********************************************************************/

   /* Delay for msec milliseconds (minimum) */
   void timer_wait_ms(TIMER_REGS_T *pTimerRegs, UNS_32 msec);

   /* Delay for usec microseconds (minimum) */
   void timer_wait_us(TIMER_REGS_T *pTimerRegs, UNS_32 usec);

#ifdef __cplusplus
}
#endif

#endif /* LPC313X_TIMER_DRIVER_H */
