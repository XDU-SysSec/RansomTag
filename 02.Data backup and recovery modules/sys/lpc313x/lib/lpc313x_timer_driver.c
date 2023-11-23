/***********************************************************************
 * $Id:: lpc313x_timer_driver.c 1355 2008-11-25 00:31:05Z pdurgesh     $
 *
 * Project: LPC313x timer driver
 *
 * Description:
 *     This file contains driver support for the LPC313x timer.
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

#include "lpc313x_timer_driver.h"
#include "lpc313x_cgu_driver.h"

/***********************************************************************
 * Timer driver package data
***********************************************************************/

/* Timer device configuration structure type */
typedef struct
{
   UNS_8 id;
   BOOL_8 init;
   UNS_8 clk_id;
   UNS_8 reset_id;
   TIMER_REGS_T *regptr;
} TIMER_CFG_T;

/* Timer driver data */
static TIMER_CFG_T g_tmr_cfg [4] =
{
   {0, 0, (UNS_8)CGU_SB_TIMER0_PCLK_ID, (UNS_8)TIMER0_PNRES_SOFT, TIMER_CNTR0},
   {1, 0, (UNS_8)CGU_SB_TIMER1_PCLK_ID, (UNS_8)TIMER1_PNRES_SOFT, TIMER_CNTR1},
   {2, 0, (UNS_8)CGU_SB_TIMER2_PCLK_ID, (UNS_8)TIMER2_PNRES_SOFT, TIMER_CNTR2},
   {3, 0, (UNS_8)CGU_SB_TIMER3_PCLK_ID, (UNS_8)TIMER3_PNRES_SOFT, TIMER_CNTR3}
};

/***********************************************************************
 * Timer driver private functions
 **********************************************************************/
/***********************************************************************
 *
 * Function: timer_ptr_to_timer_num
 *
 * Purpose: Convert a timer register pointer to a timer number
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     pTimer : Pointer to a timer register set
 *
 * Outputs: None
 *
 * Returns: The timer number (0 to 3) or -1 if register pointer is bad
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 timer_ptr_to_timer_num(TIMER_REGS_T *pTimer)
{
   INT_32 tnum = -1;

   if (pTimer == TIMER_CNTR0)
   {
      tnum = 0;
   }
   else if (pTimer == TIMER_CNTR1)
   {
      tnum = 1;
   }
   else if (pTimer == TIMER_CNTR2)
   {
      tnum = 2;
   }
   else if (pTimer == TIMER_CNTR3)
   {
      tnum = 3;
   }

   return tnum;
}

/***********************************************************************
 *
 * Function: timer_usec_to_val
 *
 * Purpose: Convert a time to a timer count value
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     clk_id : Timer clock ID
 *     usec   : Time in microseconds
 *
 * Outputs: None
 *
 * Returns: The number of required clock ticks to give the time delay
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 timer_usec_to_val(CGU_CLOCK_ID_T clk_id, UNS_32 usec, UNS_32* pcon)
{
   UNS_64 clkdlycnt;
   UNS_64 freq;

   /* Determine the value to exceed before the count reaches the desired
      delay time */
   freq = (UNS_64)cgu_get_clk_freq(clk_id);
   if ((freq > 1000000) && (pcon != NULL))
   {
      /* if timer freq is greater than 1MHz use pre-dividers */
      *pcon &= ~TM_CTRL_PS_MASK;
      if (usec > 100000)
      {
         /* use divide by 256 pre-divider for delay greater than 100 msec*/
         *pcon |= TM_CTRL_PS256;
         /* divide by 256 */
         freq = freq >> 8;
      }
      else if (usec > 1000)
      {
         /* use divide by 16 pre-divider for delay greater than 1 msec*/
         *pcon |= TM_CTRL_PS16;
         /* divide by 16 */
         freq = freq >> 4;
      }
   }

   clkdlycnt =	((freq * (UNS_64)usec) / 1000000);

   return (UNS_32)clkdlycnt;
}

/***********************************************************************
 *
 * Function: timer_delay_cmn
 *
 * Purpose: Delay for a period of microseconds
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     pTimer: Pointer to timer register set to use
 *     usec  :  the delay time in microseconds
 *
 * Outputs: None
 *
 * Returns: The clock rate of the timer in Hz, or 0 if invalid
 *
 * Notes: None
 *
 **********************************************************************/
void timer_delay_cmn(TIMER_REGS_T* pTimerRegs, UNS_32 usec)
{
   UNS_32 control = 0;
   UNS_32 clkdlycnt;
   TIMER_CFG_T *pTimer = NULL;
   INT_32 tnum = timer_ptr_to_timer_num(pTimerRegs);

   /* check if valid regs pointer is passed. If not find a free timer.*/
   if (tnum < 0)
   {
      tnum = 3;
      while (tnum >= 0)
      {
         if (g_tmr_cfg[tnum].init == FALSE)
            break;

         tnum--;
      }
   }
   /* no timer is free so simply return */
   if (tnum < 0)
   {
      return;
   }

   /* get timer device config structure */
   pTimer = &g_tmr_cfg[tnum];

   /* Enable timer system clock */
   cgu_clk_en_dis((CGU_CLOCK_ID_T)pTimer->clk_id, 1);

   /* Determine the value to exceed before the count reaches the desired
      delay time */
   clkdlycnt = timer_usec_to_val((CGU_CLOCK_ID_T)pTimer->clk_id, usec, &control);

   /* Reset timer */
   pTimer->regptr->control &= ~TM_CTRL_ENABLE;
   pTimer->regptr->load = clkdlycnt;

   /* Enable the timer in free running mode*/
   pTimer->regptr->control = control | TM_CTRL_ENABLE;

   /* Loop until terminal count matches or exceeds computed delay count */
   while (pTimer->regptr->value <= clkdlycnt);

   /* Disable timer system clock */
   cgu_clk_en_dis((CGU_CLOCK_ID_T)pTimer->clk_id, 0);

   /* Stop timer */
   pTimer->regptr->control &= ~TM_CTRL_ENABLE;
}

/***********************************************************************
 * Timer driver public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: timer_open
 *
 * Purpose: Open the timer
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     ipbase: Pointer to a timer peripheral block
 *     arg   : Not used
 *
 * Outputs: None
 *
 * Returns: The pointer to a timer config structure or NULL
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 timer_open(void *ipbase,
                  INT_32 arg)
{
   TIMER_CFG_T *pTimer;
   INT_32 tnum, tptr = (INT_32) NULL;

   /* Try to find a matching timer number based on the pass pointer */
   tnum = timer_ptr_to_timer_num((TIMER_REGS_T *) ipbase);
   if (tnum >= 0)
   {
      pTimer = &g_tmr_cfg[tnum];
      /* Has the timer been previously initialized? */
      if (pTimer->init == FALSE)
      {
         /* Timer is free */
         pTimer->init = TRUE;
         pTimer->regptr = (TIMER_REGS_T *) ipbase;

         /* Enable timer system clock */
         cgu_clk_en_dis((CGU_CLOCK_ID_T)pTimer->clk_id, 1);

         /* Setup default timer state as standard timer mode, timer
            disabled and all match and counters disabled */
         cgu_soft_reset_module((CGU_MOD_ID_T)pTimer->reset_id);
         /*
         pTimer->regptr->control &= ~TM_CTRL_ENABLE;
         pTimer->regptr->clear = 1;
         pTimer->regptr->load = 0;
         */
         /* Return pointer to specific timer structure */
         tptr = (INT_32) pTimer;
      }
   }

   return tptr;
}

/***********************************************************************
 *
 * Function: timer_close
 *
 * Purpose: Close the timer
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR to the caller as the
 *     device was not previously opened. Otherwise, disable the timers,
 *     set init to FALSE, and return _NO_ERROR to the caller.
 *
 * Parameters:
 *     devid: Pointer to timer config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
LPC_STATUS timer_close(INT_32 devid)
{
   TIMER_CFG_T *pTimer;
   LPC_STATUS status = _ERROR;

   /* Get timer device structure */
   pTimer = (TIMER_CFG_T *) devid;
   if (pTimer->init == TRUE)
   {
      /* Disable all timer fucntions */
      pTimer->regptr->control &= ~TM_CTRL_ENABLE;
      pTimer->regptr->clear = 1;
      pTimer->regptr->load = 0;

      /* Disable timer system clock */
      cgu_clk_en_dis((CGU_CLOCK_ID_T)pTimer->clk_id, 0);
   }

   return status;
}

/***********************************************************************
 *
 * Function: timer_ioctl
 *
 * Purpose: Timer configuration block
 *
 * Processing:
 *     This function is a large case block. Based on the passed function
 *     and option values, set or get the appropriate timer parameter.
 *
 * Parameters:
 *     devid: Pointer to timer config structure
 *     cmd:   ioctl command
 *     arg:   ioctl argument
 *
 * Outputs: None
 *
 * Returns: The status of the ioctl operation
 *
 * Notes: None
 *
 **********************************************************************/
LPC_STATUS timer_ioctl(INT_32 devid,
                       INT_32 cmd,
                       INT_32 arg)
{
   UNS_32 tmp;
   TIMER_CFG_T *pTimer;
   TMR_CFG_ARG_T *pCfg;
   INT_32 status = _ERROR;

   /* Get timer device structure */
   pTimer = (TIMER_CFG_T *) devid;
   if (pTimer->init == TRUE)
   {
      status = _NO_ERROR;

      switch (cmd)
      {
         case TMR_ENABLE:
            if (arg != 0)
            {
               /* Enable the timer */
               pTimer->regptr->control |= TM_CTRL_ENABLE;
            }
            else
            {
               /* Disable the timer */
               pTimer->regptr->control &= ~TM_CTRL_ENABLE;
            }
            break;

         case TMR_RESET:
            cgu_soft_reset_module((CGU_MOD_ID_T)pTimer->reset_id);
            break;

         case TMR_CLEAR_INTS:
            pTimer->regptr->clear = 1;
            break;

         case TMR_SET_CFG:
            pCfg = (TMR_CFG_ARG_T*)arg;
            tmp = 0;
            pTimer->regptr->load = timer_usec_to_val((CGU_CLOCK_ID_T)pTimer->clk_id, pCfg->usecs, &tmp);
            /* set periodic if requested */
            if (pCfg->periodic)
            {
               tmp |= TM_CTRL_PERIODIC;
            }

            if (pCfg->enable)
            {
               /* Enable the timer */
               tmp |= TM_CTRL_ENABLE;
            }
            pTimer->regptr->clear = 1;
            pTimer->regptr->control = tmp;
            break;

         case TMR_SET_LOAD:
            pTimer->regptr->load = (UNS_32) arg;
            break;

         case TMR_SET_PERIODIC_MODE:
            if (arg != 0)
               pTimer->regptr->load = (UNS_32) arg;

            pTimer->regptr->control |= TM_CTRL_PERIODIC;
            break;

         case TMR_SET_FREERUN_MODE:
            if (arg != 0)
               pTimer->regptr->load = (UNS_32) arg;

            pTimer->regptr->control &= ~TM_CTRL_PERIODIC;
            break;

         case TMR_SET_PSCALE:
            tmp = pTimer->regptr->control & ~TM_CTRL_PS_MASK;
            tmp = tmp | (arg & TM_CTRL_PS_MASK);
            pTimer->regptr->control = tmp;
            break;

         case TMR_SET_MSECS:
            arg = arg * 1000;
         case TMR_SET_USECS:
            tmp = pTimer->regptr->control;
            pTimer->regptr->load = timer_usec_to_val((CGU_CLOCK_ID_T)pTimer->clk_id, arg, &tmp);
            pTimer->regptr->control = tmp;
            break;

         case TMR_GET_STATUS:
            /* Return a timer status */
            switch (arg)
            {
               case TMR_GET_COUNT:
                  status = pTimer->regptr->value;
                  break;

               case TMR_GET_PS_SCALE:
                  status = pTimer->regptr->control & TM_CTRL_PS_MASK;
                  break;

               case TMR_GET_CLOCK:
                  status = cgu_get_clk_freq((CGU_CLOCK_ID_T)pTimer->clk_id);
                  break;

               default:
                  /* Unsupported parameter */
                  status = LPC_BAD_PARAMS;
                  break;
            }
            break;

         default:
            /* Unsupported parameter */
            status = LPC_BAD_PARAMS;
      }
   }

   return status;
}

/***********************************************************************
 *
 * Function: timer_read
 *
 * Purpose: Timer read function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid:     Pointer to timer descriptor
 *     buffer:    Pointer to data buffer to copy to
 *     max_bytes: Number of bytes to read
 *
 * Outputs: None
 *
 * Returns: Number of bytes actually read (always 0)
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 timer_read(INT_32 devid,
                  void *buffer,
                  INT_32 max_bytes)
{
   return 0;
}

/***********************************************************************
 *
 * Function: timer_write
 *
 * Purpose: Timer write function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid:   Pointer to timer descriptor
 *     buffer:  Pointer to data buffer to copy from
 *     n_bytes: Number of bytes to write
 *
 * Outputs: None
 *
 * Returns: Number of bytes actually written (always 0)
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 timer_write(INT_32 devid,
                   void *buffer,
                   INT_32 n_bytes)
{
   return 0;
}

/***********************************************************************
 *
 * Function: timer_wait_ms
 *
 * Purpose: Delay for msec milliseconds (minimum)
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     pTimerRegs: Pointer to timer register set to use
 *     msec  :  the delay time in milliseconds
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes:
 *     Use of this function will destroy any previous timer settings
 *     (for the specific timer used) and should not be used if that
 *     timer is simultaneously being used for something else.
 *
 **********************************************************************/
void timer_wait_ms(TIMER_REGS_T *pTimerRegs, UNS_32 msec)
{
   timer_delay_cmn(pTimerRegs, (msec * 1000));
}

/***********************************************************************
 *
 * Function: timer_wait_us
 *
 * Purpose: Delay for usec microseconds (minimum)
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     pTimerRegs: Pointer to timer register set to use
 *     usec  :  the delay time in microseconds
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes:
 *     Use of this function will destroy any previous timer settings
 *     (for the specific timer used) and should not be used if that
 *     timer is simultaneously being used for something else.
 *
 **********************************************************************/
void timer_wait_us(TIMER_REGS_T *pTimerRegs, UNS_32 usec)
{
   timer_delay_cmn(pTimerRegs, usec);
}
