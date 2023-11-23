/*************************************************************************
 *
 *   Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2008
 *
 *    File name   : drv_intc.h
 *    Description :  lpc313x Interrupt Controller Driver header file
 *
 *    History :
 *    1. Date        : 10.4.2009
 *       Author      : Stoyan Choynev
 *       Description : Initial Revison
 *
 *    $Revision: 31493 $
 **************************************************************************/
 
#ifndef __DRV_INTC_H
#define __DRV_INTC_H

/** include files **/

/** definitions **/

typedef enum
{
   /* main interrupts */
   IRQ_EVT_ROUTER0 = 1,      /*interrupts from Event router 0*/
   IRQ_EVT_ROUTER1,          /*interrupts from Event router 1*/
   IRQ_EVT_ROUTER2,          /*interrupts from Event router 2*/
   IRQ_EVT_ROUTER3,          /*interrupts from Event router 3*/
   IRQ_TIMER0,               /*Timer 0 IRQ */
   IRQ_TIMER1,               /*Timer 1 IRQ */
   IRQ_TIMER2,               /*Timer 2 IRQ */
   IRQ_TIMER3,               /*Timer 3 IRQ */
   IRQ_ADC,                  /*10bit ADC irq*/
   IRQ_UART,                 /*UART irq */
   IRQ_I2C0,                 /*I2C 0 IRQ */
   IRQ_I2C1,                 /*I2C 1 IRQ */
   IRQ_I2S0_OUT,             /*I2S 0 out IRQ */
   IRQ_I2S1_OUT,             /*I2S 1 out IRQ */
   IRQ_I2S0_IN,              /*I2S 0 IN IRQ */
   IRQ_I2S1_IN,              /*I2S 1 IN IRQ */
   IRQ_RSVD1,
   IRQ_LCD = 18,             /*LCD irq */
   IRQ_SPI_SMS,              /*SPI SMS IRQ */
   IRQ_SPI_TX,               /*SPI Transmit IRQ */
   IRQ_SPI_RX,               /*SPI Receive IRQ */
   IRQ_SPI_OVR,              /*SPI overrun IRQ */
   IRQ_SPI,                  /*SPI interrupt IRQ */
   IRQ_DMA,                  /*DMA irq */
   IRQ_NAND_FLASH,           /*NAND flash irq */
   IRQ_MCI,                  /*MCI irq */
   IRQ_USB,                  /*USB irq */
   IRQ_ISRAM0,               /*ISRAM0 irq */
   IRQ_ISRAM1,               /*ISRAM1 irq */
   IRQ_END_OF_INTERRUPTS = IRQ_ISRAM1
} INTERRUPT_SOURCE_T;


/* USB interrupt priority */
#define TIMER1_INTR_PRIORITY     (12)
#define UART_INTR_PRIORITY       (13)
#define DMA_INTR_PRIORITY        (14)
#define USB_INTR_PRIORITY        (15)


/** default settings **/

/** public data **/

/** public functions **/
__arm __irq void IRQ_Handler(void);
void INTC_Init(Int32U * VectorAddress);
void INTC_IRQInstall(VoidFpnt_t ISR, Int32U IRQIndex,
                     Int32U Priority, Int32U Active);
void INTC_IntEnable(Int32U IRQIndex, Int32U Enable);
#endif /* __DRV_INTC_H */

