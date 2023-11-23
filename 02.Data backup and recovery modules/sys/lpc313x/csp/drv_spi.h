/*************************************************************************
 *
 *   Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2008
 *
 *    File name   : drv_spi.h
 *    Description :  lpc313x SPI Driver header file
 *
 *    History :
 *    1. Date        : 27.3.2009
 *       Author      : Stoyan Choynev
 *       Description : Initial Revison
 *
 *    $Revision: 31493 $
 **************************************************************************/
 
#ifndef __DRV_SPI_H
#define __DRV_SPI_H

/** include files **/
#include <NXP\iolpc3130.h>
#include "arm_comm.h"

/** definitions **/
typedef enum
{
   SLAVE0 = 0,
   SLAVE1,
   SLAVE2
} SLAVE_ID;

typedef enum
{
   SPI_MODE0 = 0,
   SPI_MODE1,
   SPI_MODE2,
   SPI_MODE3,
   SSP_MODE
} SLAVE_MODE ;

typedef struct
{
   SLAVE_ID Slave;
   Int32U Clock;
   Int8U  NumberWords;
   Int8U  InterDly;
   Int8U  WordSize;
   SLAVE_MODE Mode;
   Int16U CS_Value : 1;
   Int16U CS_Dly   : 8;
} SLAVE_CNFG;

typedef struct
{
   Int32U  InterSlvDly :16;
   Int32U  T_Mode      : 1;
   Int32U  LB_Mode     : 1;
   Int32U  MS_Mode     : 1;
} SPI_CNFG;

enum SLAVE_UPDATE
{
   SLAVE_DISABLE = 0,
   SLAVE_ENABLE,
   SLAVE_SUSPEND = 3,
};

#define SPI_FIFO_SIZE 64
#define TX_FIFO_EMPTY (1<<0)
#define TX_FIFO_FULL  (1<<1)
#define RX_FIFO_EMPTY (1<<2)
#define RX_FIFO_FULL  (1<<3)
#define SPI_BUSY      (1<<4)
#define SMS_MODE_BUSY (1<<5)

/* macro */
#define SPI_CS0_HIGH  {IOCONF_SPI_M1_SET = (1<<4);\
                       IOCONF_SPI_M0_SET = (1<<4);}
#define SPI_CS0_LOW   {IOCONF_SPI_M1_SET = (1<<4);\
                       IOCONF_SPI_M0_CLR = (1<<4);}

/** default settings **/

/** public data **/

/** public functions **/
extern void SPI_configure(SPI_CNFG * spi_cnfg);
extern void Slave_configure(SLAVE_CNFG * Slave_cnfg);
extern void SPI_Enable(void);
extern void SPI_Disable(void);
extern void Slave_update(Int32U update);
extern Int32U SPI_read_fifo(SLAVE_ID Slave, void *buffer,Int32U Words);
extern Int32U SPI_write_fifo(SLAVE_ID Slave,void *buffer,Int32U Words);
extern Int32U SPI_Get_Status();
extern void SPI_rx_fifo_flush();
extern void SPI_tx_fifo_flush();
#endif /* __DRV_SPI_H */