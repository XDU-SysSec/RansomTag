/*************************************************************************
 *
 *   Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2008
 *
 *    File name   : drv_intc.c
 *    Description : LPC313x Interrupt controller driver
 *
 *    History :
 *    1. Date        : 10.4.2009 ã.
 *       Author      : Stoyan Choynev
 *       Description : Initial revistion
 *
 *    $Revision: 31493 $
 **************************************************************************/

/** include files **/
#include <NXP\iolpc3130.h>
#include <assert.h>
#include "arm_comm.h"
#include "drv_cgu.h"


/** local definitions **/
typedef struct _IntrVecTable_t
{
   VoidFpnt_t  pIntrFunc;
   Int32U      IntrPriority;
} IntrVecTable_t, *pIntrVecTable_t;
/** default settings **/

/** external functions **/

/** external data **/
extern Int32U  __vector[];
extern Int32U  __vector_end[];

/** internal functions **/
void No_IRQ(void);
void INTC_Copy_Vector(Int32U * Dest);
/** public data **/

/** private data **/
#pragma data_alignment=2048
static IntrVecTable_t IntTbl[ISRAM1_MRC_FINISHED+1];
/** public functions **/
/*************************************************************************
 * Function Name: IRQ_Handler
 * Parameters: None
 *
 * Return: None
 *
 * Description: Nested IRQ handler.
 *
 *************************************************************************/
__arm __nested __irq void IRQ_Handler(void)
{
   /*Save current priority mask*/
   volatile Int32U PriorityMask = INT_PRIORITYMASK_0;
   /*read IRQ interrupt vector*/
   IntrVecTable_t * IntVector = (IntrVecTable_t *)INT_VECTOR_0;
   /*Update interrup priority mask*/
   INT_PRIORITYMASK_0 = IntVector->IntrPriority;
   /*enable interrupts*/
   __enable_irq();
   /*Call Interrupt Service Routine*/
   (IntVector->pIntrFunc)();
   /*disable interrupts*/
   __disable_irq();
   /*restore priority mask*/
   INT_PRIORITYMASK_0 = PriorityMask;
}
/*************************************************************************
 * Function Name: INTC_Init
 * Parameters:Int32U * VectorAddress - Pointer to RAM (SDRAM)
 *                                     array. ARM interrupt vectors
 *                                     will be copyed at this address
 * Return: None
 *
 * Description: Initialize LPC313x Interrupt Controller
 *
 *************************************************************************/
void INTC_Init(Int32U * VectorAddress)
{
   /*Enable Intrrupt Controller Clocks*/
   CGU_Run_Clock(AHB_TO_INTC_CLK);
   CGU_Run_Clock(INTC_CLK);
   /*Interrupt vector register init. Only IRQ
     table is used. FIQ set to zero.*/
   INT_VECTOR_0 = (Int32U)IntTbl;
   INT_VECTOR_1 = 0;
   /*Set priority mask to low i.e. do not mask
     any interrupts.*/
   INT_PRIORITYMASK_0 = 0;
   INT_PRIORITYMASK_1 = 0;
   /*Interrupt init*/
   IntTbl[0].pIntrFunc = No_IRQ;
   IntTbl[0].IntrPriority = 0;

   for(Int32U i = CASCADED_IRQ_0; ISRAM1_MRC_FINISHED >= i; i++ )
   {
      /*IRQ table init*/
      IntTbl[i].pIntrFunc = No_IRQ;
      IntTbl[i].IntrPriority = 0;
      /*Disable all interrupts. Set all as IRQ,
       active high, priroity 0 (interrupt is masked).*/
      *(&INT_REQUEST_1-1+i) = (1UL << 29) |  // clear software interrupt
                              (1UL << 28) |  // Update Priority
                              (1UL << 27) |  // Update target
                              (1UL << 26) |  // Update interrupt enable
                              (1UL << 25) |  // Update interrupt active level
                              0;
   }
   /*Copy Arm Vectors*/
   INTC_Copy_Vector(VectorAddress);
}

/*************************************************************************
 * Function Name: INTC_IRQInstall
 * Parameters:VoidFpnt_t ISR - Interrup Service Rotune
 *            Int32U IRQIndex - IRQ Number
 *            Int32U Priority - Priority level from 1(low) to 15(high)
 *            Int32U Active   - Active level 0 - high, 1 - low
 * Return: None
 *
 * Description: Install IRQ
 *
 *************************************************************************/
void INTC_IRQInstall(VoidFpnt_t ISR, Int32U IRQIndex,
                     Int32U Priority, Int32U Active)
{
   assert((CASCADED_IRQ_0 <= IRQIndex) && (ISRAM1_MRC_FINISHED >= IRQIndex));
   assert((1 <= Priority)&&(15 >= Priority));
   /*Update Interrupt Table*/
   IntTbl[IRQIndex].pIntrFunc = ISR;
   IntTbl[IRQIndex].IntrPriority = Priority;
   /*Set INT_REQUEST*/
   *(&INT_REQUEST_1-1+IRQIndex) = (1UL << 29) |  // clear software interrupt
                                  (1UL << 28) |  // Update Priority
                                  (1UL << 27) |  // Update target
                                  (1UL << 26) |  // Update interrupt enable
                                  (1UL << 25) |  // Update interrupt active level
                                  ((Active)?(1<<17):0) |
                                  Priority;
}
/*************************************************************************
 * Function Name: INTC_IntEnable
 * Parameters:Int32U IRQIndex - IRQ Number
 *            Int32U Enable - 0 - Disable, 1- Enable)
 * Return: None
 *
 * Description: Enable or Disable IRQ
 *
 *************************************************************************/
void INTC_IntEnable(Int32U IRQIndex, Int32U Enable)
{
   *(&INT_REQUEST_1-1+IRQIndex) = (1UL << 26) |
                                  ((Enable)?(1<<16):0);
}

/** private functions **/
void No_IRQ(void)
{
}

void INTC_Copy_Vector(Int32U * Dest)
{
   Int32U * Src = __vector  ;
   while( Src <= __vector_end)
   {
      * Dest++ = * Src++;
   }
}
