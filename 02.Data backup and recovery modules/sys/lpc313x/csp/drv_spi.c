/*************************************************************************
 *
 *   Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2008
 *
 *    File name   : drv_spi.c
 *    Description : lpc313x SPI Driver
 *
 *    History :
 *    1. Date        : 26.3.2009
 *       Author      : Stoyan Choynev
 *       Description : Initial Revision
 *
 *    $Revision: 33745 $
 **************************************************************************/

/** include files **/
#include "drv_spi.h"
/** local definitions **/
/** default settings **/
#define SPI_MAX_CLK_R   254*(1+255)
#define SPI_MIN_CLK_R   2
#define SPI_MAIN_CLOCK  90MHZ

/** external functions **/

/** external data **/

/** internal functions **/

/** public data **/

/** private data **/

/** public functions **/
/*************************************************************************
 * Function Name: SPI_configure
 * Parameters:SPI_CNFG * spi_cnfg
 *
 * Return: None
 *
 * Description: None
 *
 *************************************************************************/
void SPI_configure(SPI_CNFG * spi_cnfg)
{
   /*Reset SPI module*/
   SPI_CONFIG_bit.SRST = 1;
   /*Configure Inter Slave delay*/
   SPI_CONFIG_bit.ISD = spi_cnfg->InterSlvDly;
   /*Tnasmision Mode*/
   SPI_CONFIG_bit.TM = spi_cnfg->T_Mode;
   /*Loop Back mode*/
   SPI_CONFIG_bit.LB = spi_cnfg->LB_Mode;
   /*Master or slave mode*/
   SPI_CONFIG_bit.MST = spi_cnfg->MS_Mode;
   /*If Slave mode*/
   if(spi_cnfg->MS_Mode) SPI_SLAVE_ENABLE = 0; /*Clear Slave Enable*/
}
/*************************************************************************
 * Function Name: Slave_configure
 * Parameters:SLAVE_CNFG * Slave_cnfg
 *
 * Return: None
 *
 * Description: None
 *
 *************************************************************************/
void Slave_configure(SLAVE_CNFG * Slave_cnfg)
{
   Int32U * SlaveSetting1 = (Int32U *)(&SPI_SLV0_SETTINGS1+(2*Slave_cnfg->Slave));
   Int32U * SlaveSetting2 = (Int32U *)(&SPI_SLV0_SETTINGS2+(2*Slave_cnfg->Slave));
   /*configure clock Divisors*/
   Int32U clock_ratio = (SPI_MAIN_CLOCK + Slave_cnfg->Clock/2)/Slave_cnfg->Clock;
   Int32U  div2,tmp = 0;

   /*Corect Clock Ratio*/
   if(SPI_MIN_CLK_R > clock_ratio) clock_ratio = SPI_MIN_CLK_R;
   if(SPI_MAX_CLK_R < clock_ratio) clock_ratio = SPI_MAX_CLK_R;
   /*Set Clock Divisors*/
   div2 = (((clock_ratio - 1)/512)+1)*2;
   tmp = ((clock_ratio + div2/2)/div2)-1;
   tmp |= div2<<8;
   /*Words in fifo for the slave*/
   tmp |= (Slave_cnfg->NumberWords)<<16;
   /*Delay*/
   tmp |= Slave_cnfg->InterDly<<24;
   *SlaveSetting1  = tmp;
   tmp = 0;
   /*Word trasfer size*/
   tmp |= Slave_cnfg->WordSize;
   /*Mode*/
   tmp |= (Slave_cnfg->Mode)<<5;
   /*Chip Select*/
   tmp |= (Slave_cnfg->CS_Value)<<8;
   tmp |= (Slave_cnfg->CS_Dly)<<9;
   *SlaveSetting2  = tmp;
}

/*************************************************************************
 * Function Name: SPI_Enable
 * Parameters:None
 *
 * Return: None
 *
 * Description: Enable SPI module
 *
 *************************************************************************/
void SPI_Enable(void)
{
   SPI_CONFIG_bit.ENA = 1;
}
/*************************************************************************
 * Function Name: SPI_Disable
 * Parameters:None
 *
 * Return: None
 *
 * Description: Disable SPI module
 *
 *************************************************************************/
void SPI_Disable(void)
{
   SPI_CONFIG_bit.ENA = 0;
}

/*************************************************************************
 * Function Name: Slave_update
 * Parameters:Int32U update
 *
 * Return: None
 *
 * Description: write new value in Slave_eable and set
 *              SPI Slave Update Bit
 *************************************************************************/
void Slave_update(Int32U update)
{
   SPI_SLAVE_ENABLE_bit.SENA = update;
   SPI_CONFIG_bit.UENA = 1;
}

/*************************************************************************
 * Function Name: SPI_read_fifo
 * Parameters:SLAVE_ID Slave - Slave number
 *             void *buffer  - buffer pointer
 *             Int32U Words  - number of words
 *
 * Return: Int32U  - number of words
 *
 * Description: reads words from rx fifo
 *
 *************************************************************************/
Int32U SPI_read_fifo(SLAVE_ID Slave, void *buffer,Int32U Words)
{
   Int32U count = 0;
   Int16U *data16 = NULL;
   Int8U *data8 = NULL;
   volatile __spi_slv_settings2_bits * SlaveSetting2 = (__spi_slv_settings2_bits *)(&SPI_SLV0_SETTINGS2+(2*Slave));

   /*determine sample width */
   if(8 < SlaveSetting2->WS)
   {
      data16 = (Int16U *)buffer;
   }
   else
   {
      data8 = (Int8U *)buffer;
   }
   /* Loop until transmit ring buffer is full or until
     expires */
   while ((0 != Words) && (!SPI_STATUS_bit.rx_fifo_empty))
   {
      if (data16 == NULL)
      {
         *data8 = (Int8U)SPI_FIFO_DATA;
         data8++;
      }
      else
      {
         *data16 = (Int16U)SPI_FIFO_DATA;
         data16++;
      }

      /* Increment data count and decrement buffer size count */
      count++;
      Words--;
   }

   return count;
}
/*************************************************************************
 * Function Name: SPI_write_fifo
 * Parameters:SLAVE_ID Slave - Slave number
 *             void *buffer  - buffer pointer
 *             Int32U Words  - number of words
 *
 * Return: Int32U  - number of words
 *
 * Description: writes words in tx fifo
 *
 *************************************************************************/
Int32U SPI_write_fifo(SLAVE_ID Slave,void *buffer,Int32U Words)
{

   Int32U count = 0;
   Int16U *data16 = NULL;
   Int8U *data8 = NULL;
   volatile __spi_slv_settings2_bits * SlaveSetting2 = (__spi_slv_settings2_bits *)(&SPI_SLV0_SETTINGS2+(2*Slave));


   /*determine sample width */
   if(8 < SlaveSetting2->WS)
   {
      data16 = (Int16U *)buffer;
   }
   else
   {
      data8 = (Int8U *)buffer;
   }
   /* Loop until transmit ring buffer is full or until n_bytes
      expires */
   while ((0 != Words) && (!SPI_STATUS_bit.tx_fifo_full))
   {
      if (data16 == NULL)
      {
         SPI_FIFO_DATA = (Int32U) * data8;
         data8++;
      }
      else
      {
         SPI_FIFO_DATA = (Int32U) * data16;
         data16++;
      }

      /* Increment data count and decrement buffer size count */
      count++;
      Words--;
   }

   return count;
}

Int32U SPI_Get_Status(void)
{
   return SPI_STATUS;
}

void SPI_rx_fifo_flush(void)
{
   volatile Int32U Dummy;

   while(!SPI_STATUS_bit.rx_fifo_empty)
   {
      Dummy = SPI_FIFO_DATA;
   }
}

void SPI_tx_fifo_flush(void)
{
   SPI_TX_FIFO_FLUSH = 1;
}

/** private functions **/
