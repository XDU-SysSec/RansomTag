/*************************************************************************
 *
 *    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2009
 *
 *    File name   : uart.h
 *    Description : UARTs module header
 *
 *    History :
 *    1. Date        : August 26, 2009
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *    $Revision: 31493 $
 **************************************************************************/

#ifndef __UART_H
#define __UART_H

#ifdef UART_GLOBAL
#define UART_EXTERN
#else
#define UART_EXTERN  extern
#endif

#define UART_FIFO_SIZE        32

#define UART_MAX_BAUD_RATE    256000

#define RLS_INTR_ID           0x6
#define RDA_INTR_ID           0x4
#define CDI_INTR_ID           0xC
#define THRE_INTR_ID          0x2
#define MODEM_INTR_ID         0x0

#define RLS_ReceiverDataReady (1UL<<0)
#define RLS_OverrunError      (1UL<<1)
#define RLS_ParityError       (1UL<<2)
#define RLS_FramingError      (1UL<<3)
#define RLS_BreakInterruptr   (1UL<<4)
#define RLS_TranHoldRegEmpty  (1UL<<5)
#define RLS_TranShiftRegEmpty (1UL<<6)
#define RLS_RxFifoError       (1UL<<7)

// Define UARTs
typedef enum _UartNum_t
{
   UART_0 = 0
} UartNum_t;

typedef enum _UartParity_t
{
   UART_ODD_PARITY = 0, UART_EVEN_PARITY,
   UART_FORCE_1_PARITY, UART_FORCE_0_PARITY,
   UART_NO_PARITY
} UartParity_t;

typedef enum _UartStopBits_t
{
   UART_ONE_STOP_BIT = 0, UART_TWO_STOP_BIT,
} UartStopBits_t;

typedef enum _UartWordWidth_t
{
   UART_WORD_WIDTH_5 = 0, UART_WORD_WIDTH_6,
   UART_WORD_WIDTH_7, UART_WORD_WIDTH_8
} UartWordWidth_t;

typedef struct _UartFifo_t
{
   Int32U PushIndx;
   Int32U PopIndx;
   Int8U  Buffer[UART_FIFO_SIZE];
} UartFifo_t, *pUartFifo_t;

typedef union _UartModemLineState_t
{
   Int32U Data;
   struct
   {
      Int32U  bDTR          : 1;
      Int32U  bRTS          : 1;
      Int32U                :30;
   };
} UartModemLineState_t, *pUartModemLineState_t;

typedef struct _UartLineCoding_t
{
   Int32U          dwDTERate;
   UartStopBits_t  bStopBitsFormat;
   UartParity_t    bParityType;
   UartWordWidth_t bDataBits;
} UartLineCoding_t, * pUartLineCoding_t;

typedef union _UartLineEvents_t
{
   Int32U Data;
   struct
   {
      Int32U              : 1;
      Int32U  bOE         : 1;
      Int32U  bPE         : 1;
      Int32U  bFE         : 1;
      Int32U  bBI         : 1;
      Int32U              :27;
   };
} UartLineEvents_t, *pUartLineEvents_t;

typedef union _UartModemEvents_t
{
   Int32U Data;
   struct
   {
      Int32U  bDCTS       : 1;
      Int32U  bDDSR       : 1;
      Int32U  bDRI        : 1;
      Int32U  bDDCD       : 1;
      Int32U  bCTS        : 1;
      Int32U  bDSR        : 1;
      Int32U  bRI         : 1;
      Int32U  bDCD        : 1;
      Int32U              :24;
   };
} UartModemEvents_t, *pUartModemEvents_t;

extern Int32U SYS_GetFpclk(Int32U Periphery);

/*************************************************************************
 * Function Name: UartInit
 * Parameters: Int32U IrqPrio
 *
 * Return: Boolean
 *
 * Description: Init UART
 *
 *************************************************************************/
Boolean UartInit(Int32U IrqPrio);

/*************************************************************************
 * Function Name: UartSetLineCoding
 * Parameters:  UartLineCoding_t pUartCoding
 *
 * Return: None
 *
 * Description: Init UART Baud rate, Word width, Stop bits, Parity type
 *
 *************************************************************************/
void UartSetLineCoding(UartLineCoding_t UartCoding);

/*************************************************************************
 * Function Name: UartRead
 * Parameters:   pInt8U pBuffer, Int32U BufferSize
 *
 * Return: Int32U
 *
 * Description: Read received data from UART.
 *              Return number of readied characters
 *
 *************************************************************************/
Int32U UartRead( pInt8U pBuffer, Int32U BufferSize);

/*************************************************************************
 * Function Name: UartWrite
 * Parameters:  UartNum_t Uart, pInt8U pBuffer, Int32U BufferSize
 *
 * Return: Int32U
 *
 * Description: Write a data to UART. Return number of successful
 *  transmitted bytes
 *
 *************************************************************************/
Int32U UartWrite(pInt8U pBuffer, Int32U BufferSize);

/*************************************************************************
 * Function Name: UartGetUartEvents
 * Parameters: None
 *
 * Return: UartLineEvents_t
 *
 * Description: Get Uart Line events (PE,OE, FE, BI)
 *
 *************************************************************************/
UartLineEvents_t UartGetUartLineEvents (void);

/*************************************************************************
 * Function Name: UartSetUartLineState
 * Parameters:Boolean Break
 *
 * Return: none
 *
 * Description: Set Uart Break Event
 *
 *************************************************************************/
void UartSetUartLineState (Boolean Break);

#include <stdio.h>
#include <string.h>

#ifdef NDEBUG
#define PRINTF(...)
#define GETCH(c)
#else
/* output for debug only. It impacts USB INT and speed! */
#define PRINTF(...) {sprintf(s,__VA_ARGS__); UartWrite((unsigned char *)s, strlen(s));}
UART_EXTERN char s[512];

#define GETCH(c)   UartRead((unsigned char *)(&(c)), 1)
#endif

#endif // __UART_H
