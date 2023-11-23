/*************************************************************************
 *
 *    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2009
 *
 *    File name   : uart.c
 *    Description : UARTs module
 *
 *    History :
 *    1. Date        : August 26, 2009
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *    $Revision: 31493 $
 **************************************************************************/
 
#define UART_GLOBAL
#include <stdlib.h>
#include "arm_comm.h"
#include "drv_cgu.h"
#include "drv_intc.h"
#include "drv_uart.h"

pUartFifo_t pUart0RxFifo; // Pointer to a FIFO Buffer of the UART0 Receive
pUartFifo_t pUart0TxFifo; // Pointer to a FIFO Buffer of the UART0 Transmit

// Hold UART0 Evens (PE, BI, FE, OE)
UartLineEvents_t Uart0LineEvents;

/*************************************************************************
 * Function Name: FifoPush
 * Parameters: pUartFifo_t Fifo, Int8U Data
 *
 * Return: Boolean
 *
 * Description: Push a char in a FIFO. Return TRUE when push is successful
 *  or FALSE when the FIFO is full.
 *
 *************************************************************************/
static Boolean FifoPush(pUartFifo_t Fifo, Int8U Data)
{
   Int32U IndxTmp;

   // calculate next push index
   IndxTmp = Fifo->PushIndx + 1;
   IndxTmp = IndxTmp % UART_FIFO_SIZE;

   // Check FIFO state
   if (IndxTmp == Fifo->PopIndx)
   {
      // The FIFO is full
      return(FALSE);
   }
   // Push the data
   Fifo->Buffer[Fifo->PushIndx] = Data;
   // Updating the push's index
   Fifo->PushIndx = IndxTmp;
   return(TRUE);
}

/*************************************************************************
 * Function Name: FifoPop
 * Parameters: pUartFifo_t Fifo, Int8U Data
 *
 * Return: Boolean
 *
 * Description: Pop a char from a FIFO. Return TRUE when pop is successful
 *  or FALSE when the FIFO is empty.
 *
 *************************************************************************/
static Boolean FifoPop(pUartFifo_t Fifo, pInt8U pData)
{
   Int32U IndxTmp;

   // Check FIFO state
   if (Fifo->PushIndx == Fifo->PopIndx)
   {
      // The FIFO is empty
      return(FALSE);
   }
   // Calculate the next pop index
   IndxTmp = Fifo->PopIndx + 1;
   IndxTmp = IndxTmp % UART_FIFO_SIZE;
   // Pop the data
   *pData = Fifo->Buffer[Fifo->PopIndx];
   // Updating of the pop's index
   Fifo->PopIndx = IndxTmp;
   return(TRUE);
}


/*************************************************************************
 * Function Name: Uart0Isr
 * Parameters: none
 *
 * Return: none
 *
 * Description: UART 0 interrupt routine
 *
 *************************************************************************/
static
void Uart0Isr(void)
{
   Int32U UartIntId = UART_IIR, LineStatus, Counter;
   Int8U Data;
   // Recognize the interrupt event
   switch (UartIntId & 0xF)
   {
      case RLS_INTR_ID: // Receive Line Status
      case CDI_INTR_ID: // Character Time-out Indicator
      case RDA_INTR_ID: // Receive Data Available
         // Read the line state of the UART
         LineStatus = UART_LSR;
         do
         {
            if(LineStatus & RLS_OverrunError)
            {
               // Overrun Error
               Uart0LineEvents.bOE = TRUE;
            }
            Data = UART_RBR;
            if (LineStatus & RLS_BreakInterruptr)
            {
               // Break Indicator
               Uart0LineEvents.bBI = TRUE;
            }
            else if (LineStatus & RLS_FramingError)
            {
               // Framing Error
               Uart0LineEvents.bFE = TRUE;
            }
            else if (LineStatus & RLS_ParityError)
            {
               // Parity Error
               Uart0LineEvents.bPE = TRUE;
            }
            // Push a new data into the receiver buffer
            if(!FifoPush(pUart0RxFifo,Data))
            {
               // the FIFO is full
               Uart0LineEvents.bOE = TRUE;
               break;
            }
            // Read the line state of the UART
            LineStatus = UART_LSR;
         }
         while(LineStatus & RLS_ReceiverDataReady); // Is the hardware FIFO is empty?
         break;
      case THRE_INTR_ID:  // THRE Interrupt
         // Tx UART FIFO size - 1
         // Fill whole hardware transmit FIFO
         for (Counter = 15; Counter; --Counter)
         {
            // Pop a data from the transmit buffer
            if(!FifoPop(pUart0TxFifo,&Data))
            {
               // The tx software FIFO is empty
               break;
            }
            UART_THR = Data;
         }
         break;
   }
}

/*************************************************************************
 * Function Name: UartInit
 * Parameters: Int32U IrqPrio
 *
 * Return: Boolean
 *
 * Description: Init UART
 *
 *************************************************************************/
Boolean UartInit(Int32U IrqPrio)
{
   volatile Int8U Tmp;

   pUart0RxFifo = (pUartFifo_t)malloc(sizeof(UartFifo_t));

   if(pUart0RxFifo == NULL)
   {
      return(FALSE);
   }
   pUart0TxFifo = (pUartFifo_t)malloc(sizeof(UartFifo_t));
   if(pUart0TxFifo == NULL)
   {
      free(pUart0RxFifo);
      return(FALSE);
   }
   // Init receive and transmit FIFOs
   pUart0RxFifo->PopIndx = pUart0RxFifo->PushIndx = \
                           pUart0TxFifo->PopIndx = pUart0TxFifo->PushIndx = 0;

   // Enable UART0
   CGU_Run_Clock(UART_APB_CLK);
   CGU_Run_Clock(UART_U_CLK);

   UART_LCR = 0x03; // Word Length =8, no parity , 1 stop
   UART_FCR = 0x7;  // Enable and Clear the UART0 FIFO, Set RX FIFO interrupt level - 1 char
   Tmp = UART_IER;  // Clear pending interrupts
   // enable RBR Interrupt, THRE Interrupt, RX Line Status Interrupt
   UART_IER = 0x07;

   /*Install Interrupt Service Routine,
   Priority 3*/
   INTC_IRQInstall(Uart0Isr, IRQ_UART, IrqPrio, 0);
   /*Enable Timer 1 interrupt*/
   INTC_IntEnable(IRQ_UART, 1);

   return(TRUE);
}


/*************************************************************************
 * Function Name: UartCalcDivider
 * Parameters:  Int32U Freq, Int32U Baud
 *              pInt32U pDiv, pInt32U pAddDiv, pInt32U pMul
 *
 * Return: None
 *
 * Description: Calculate the coefficients of the UART baudrate generator
 *
 *************************************************************************/
static
void UartCalcDivider(Int32U Freq, Int32U Baud,
                     pInt32U pDiv, pInt32U pAddDiv, pInt32U pMul)
{
   Int32U Temp, Error = (Int32U)-1;
   Int32U K1, K2, K3, Baudrate;
   Int32U DivTemp, MulTemp, AddDivTemp;

   //
   for(MulTemp = 1; MulTemp < 16; ++MulTemp)
   {
      K1 = Freq*MulTemp;
      for(AddDivTemp = 1; AddDivTemp < 16; ++AddDivTemp)
      {
         K3 = (MulTemp + AddDivTemp)<<4;
         K2 =  K3 * Baud;
         DivTemp = K1/K2;
         // if DIVADDVAL>0, UnDL must be UnDL >= 0x0002 or the UART will
         // not operate at the desired baud-rate!
         if(DivTemp < 2)
         {
            continue;
         }
         Baudrate = DivTemp * K3;
         Baudrate = K1/Baudrate;
         Temp = (Baudrate > Baud)? \
                (Baudrate - Baud): \
                (Baud - Baudrate);
         if (Temp < Error)
         {
            Error = Temp;
            *pDiv = DivTemp;
            *pMul = MulTemp;
            *pAddDiv = AddDivTemp;
            if(Error == 0)
            {
               return;
            }
         }
      }
   }
}

/*************************************************************************
 * Function Name: UartSetLineCoding
 * Parameters:  UartLineCoding_t pUartCoding
 *
 * Return: None
 *
 * Description: Init UART Baud rate, Word width, Stop bits, Parity type
 *
 *************************************************************************/
void UartSetLineCoding(UartLineCoding_t UartCoding)
{
   Int32U Mul, Div, AddDiv, Freq;

   // Check parameters
   if ((UartCoding.dwDTERate == 0) || (UartCoding.dwDTERate > UART_MAX_BAUD_RATE))
   {
      return;
   }

   Freq = (12ul MHZ);
   UartCalcDivider(Freq,UartCoding.dwDTERate,&Div,&AddDiv,&Mul);
   UART_LCR_bit.WdLenSel   = UartCoding.bDataBits;
   UART_LCR_bit.StopBitNum = UartCoding.bStopBitsFormat;
   UART_LCR_bit.ParEn      = (UartCoding.bParityType == UART_NO_PARITY)?0:1;
   UART_LCR_bit.ParEven    =  UartCoding.bParityType    & 1;
   UART_LCR_bit.ParStick   = (UartCoding.bParityType>>1)& 1;
   UART_LCR_bit.DLAB = 1;
   UART_DLL = Div & 0xFF;
   UART_DLM = (Div >> 8) & 0xFF;
   UART_FDR = AddDiv + (Mul << 4);
   UART_LCR_bit.DLAB = 0;
}

/*************************************************************************
 * Function Name: UartRead
 * Parameters:  pInt8U pBuffer, Int32U BufferSize
 *
 * Return: Int32U
 *
 * Description: Read received data from UART.
 *              Return number of readied characters
 *
 *************************************************************************/
Int32U UartRead(pInt8U pBuffer, Int32U BufferSize)
{
   Int32U Count;
   pUartFifo_t pUartFifo;


   pUartFifo= pUart0RxFifo;

   for (Count = 0; Count < BufferSize; ++Count)
   {
      if(!FifoPop(pUartFifo,pBuffer+Count))
      {
         break;
      }
   }
   return(Count);
}

/*************************************************************************
 * Function Name: UartWrite
 * Parameters:  pInt8U pBuffer, Int32U BufferSize
 *
 * Return: Int32U
 *
 * Description: Write a data to UART. Return number of successful
 *  transmitted bytes
 *
 *************************************************************************/
Int32U UartWrite(pInt8U pBuffer, Int32U BufferSize)
{
   Int32U Count = 0;
   pUartFifo_t pUartFifo;
   Int32U save;

   pUartFifo= pUart0TxFifo;

   while(Count != BufferSize)
   {
      volatile pInt8U pUartTxReg;
      save = __get_interrupt_state();
      __disable_interrupt();
      if((pUartFifo->PushIndx == pUartFifo->PopIndx))
      {
         // The Tx FIFO is empty
         pUartTxReg = (pInt8U)&UART_THR;
         if(UART_LSR_bit.THRE)
         {
            *pUartTxReg = *(pBuffer+Count);
            ++Count;
         }
      }
      /*
      for ( ; Count < BufferSize; ++Count)
      {
         if(!FifoPush(pUartFifo,*(pBuffer+Count)))
         {
            break;
         }
      }
      */
      __set_interrupt_state(save);
   }
   return(Count);
}

/*************************************************************************
 * Function Name: UartGetUartEvents
 * Parameters:
 *
 * Return: UartLineEvents_t
 *
 * Description: Get Uart Line events (PE,OE, FE, BI)
 *
 *************************************************************************/
UartLineEvents_t UartGetUartLineEvents (void)

{
   UartLineEvents_t  LineEvents;

   LineEvents.Data = 0;

   AtomicExchange(LineEvents.Data,&Uart0LineEvents.Data);

   return(LineEvents);
}

/*************************************************************************
 * Function Name: UartSetUartLineState
 * Parameters:  Boolean Break
 *
 * Return: none
 *
 * Description: Set Uart Break Event
 *
 *************************************************************************/
void UartSetUartLineState (Boolean Break)
{
   UART_LCR_bit.BrkCtrl  = Break;
}

