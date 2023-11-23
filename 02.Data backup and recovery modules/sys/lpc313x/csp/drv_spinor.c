/*************************************************************************
 *
 *   Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2008
 *
 *    File name   : drv_spinor.c
 *    Description : Atmel SPI NOR Flash driver
 *
 *    History :
 *    1. Date        : 27.3.2009
 *       Author      : Stoyan Choynev
 *       Description : Initial Revision
 *
 *    $Revision: 31493 $
 **************************************************************************/

/** include files **/
#include "drv_spi.h"
#include "drv_cgu.h"
#include "drv_spinor.h"
/** local definitions **/
#define CMD_RDID        0x9F        /* read device ID */
#define CMD_RDSR        0xD7        /* read status register */
#define CMD_FAST_READ   0x0B        /* read data (extra command setup time allows higher SPI clock) */

#define CMD_SE          0x7C        /* sector erase */
#define CMD_BE          0x50        /* block erase */
#define CMD_PE          0x81        /* page erase */

#define CMD_DP          0xB9        /* deep power down */
#define CMD_RES         0xAB        /* release from deep power down */

#define CMD_B1W         0x84        /* write buffer 1*/
#define CMD_B1MM        0x88        /* buffer 1 to Main Memory without Built-in Erase*/
#define CMD_B1ME        0x83        /* buffer 1 to Main Memory with Built-in Erase*/

#define CMD_MMB1        0x53        /* main memory page to Buffer 1 */
/* status register defines */
#define STATUS_RDY      (1 << 7)
#define STATUS_COMP     (1 << 6)
#define STATUS_ID       (0xF<<2)
#define STATUS_PROTECT  (1 << 1)
#define STATUS_P2P      (1 << 0)
#define NUMDATAFLASH    (sizeof(at45Devices) / sizeof(At45Desc))

typedef struct
{
   /* dataflash page number */
   Int32U pageNumber;
   /* indicate if power-of-2 binary page supported */
   Int32U hasBinaryPage;
   /* dataflash page size */
   Int32U pageSize;
   /* page offset in command*/
   Int32U pageOffset;
   /* Dataflash ID */
   Int8U id;

} At45Desc;

/** default settings **/

/** external functions **/
extern void Dly_us(Int32U Dly);
/** external data **/

/** internal functions **/
Int32U Page2Address(Int32U Page, Int8U * dAddr);

/** public data **/

/** private data **/
static const At45Desc at45Devices[] =
{
   {  512,  1, 264,   9, 0x0C},
   { 1024,  1, 264,   9, 0x14},
   { 2048,  1, 264,   9, 0x1C},
   { 4096,  1, 264,   9, 0x24},
   { 4096,  1, 528,  10, 0x2C},
   { 8192,  1, 528,  10, 0x34},
   { 8192,  1, 1056, 11, 0x3C},
   {16384,  1, 1056, 11, 0x10},
   {16384,  1, 2112, 12, 0x18},
   {32768,  1, 2112, 12, 0x20}
};

__no_init static At45Desc * Device;
__no_init static Int32U Power2Page;
/** public functions **/
/*************************************************************************
 * Function Name: InitSPINOR
 * Parameters: None
 *
 * Return:  Int32U
 *          0 - O.K.
 *          1 - Error
 * Description: Init Atmel SPI NOR Flash
 *
 *************************************************************************/
Int32U InitSPINOR(void)
{
   SPI_CNFG SPI_cnfg;
   SLAVE_CNFG Slave_cnfg;
   Int8U Status;
   Int8U ManufID[3];
   Device = NULL;
   /*Enable SPI Clocks*/
   CGU_Run_Clock(SPI_PCLK);
   CGU_Run_Clock(SPI_PCLK_GATED);
   CGU_Run_Clock(SPI_CLK);
   CGU_Run_Clock(SPI_CLK_GATED);
   /* Enable TMR0 Clock. Used for Delay */
   CGU_Run_Clock(TIMER0_PCLK);
   /*Chip select connect to IOCONFIG module
    and set High*/
   SPI_CS0_HIGH;
   /*Inter Delay time should not matter in
   Normal Transmit Mode*/
   SPI_cnfg.InterSlvDly = 1;
   SPI_cnfg.LB_Mode = 0;       /*No loop back*/
   SPI_cnfg.MS_Mode = 0;       /*Master Mode*/
   SPI_cnfg.T_Mode = 0;        /*Normal Transmit mode*/
   /*Configure SPI module*/
   SPI_configure(&SPI_cnfg);
   Slave_cnfg.Slave = SLAVE0;  /*SPI NOR on CS0*/
   Slave_cnfg.Clock = 35MHZ;   /*Max NOR clock 45MHz*/
   Slave_cnfg.InterDly = 0;    /*No delay*/
   /*Nomber Words is not used in
   Normal Transmit Mode*/
   Slave_cnfg.NumberWords = SPI_FIFO_SIZE - 1;
   Slave_cnfg.WordSize = 8-1;  /*8-bit mode*/
   /*Chip Select is controlled by GPIO and
   this are not matter fields*/
   Slave_cnfg.CS_Value = 0;
   Slave_cnfg.CS_Dly = 0;
   /*SPI mode.*/
   Slave_cnfg.Mode = SPI_MODE0;
   Slave_configure(&Slave_cnfg);
   Slave_update((SLAVE_ENABLE)<<0); /*Enable Slave 0*/
   SPI_Enable();                    /*Enable SPI*/
   /*Release from Deep Power Down*/
   SPINOR_Res_DP();
   Dly_us(100);
   /*Read Manufacture ID*/
   SPINOR_Get_Device_ID(ManufID);
   /*If it is not Armel return Error*/
   if(0x1F != ManufID[0]) return 1;

   do
   {
      Status = SPINOR_Read_Status();
   }
   while (!(Status & STATUS_RDY));
   /*Device is in Power of 2 page mode
     Power2Page = 1*/
   Power2Page = Status & STATUS_P2P;
   /*device ID*/
   Int32U id = Status & STATUS_ID;
   /*Do we support this device?*/
   for(Int32U i = 0; NUMDATAFLASH > i; i++)
   {
      if(at45Devices[i].id == id)
      {
         /*We support this device!
           Get its descriptor*/
         Device = (At45Desc *)&at45Devices[i];
      }
   }
   /*If we do not have device descriptor
     return error*/
   if(NULL == Device) return 1;
   /**/
   return 0;
}

/*************************************************************************
 * Function Name: SPINOR_InOut
 * Parameters: Int8U * DataOut - Out Data pointer
 *             Int32U OutSize - Out Data Size
 *             Int8U * DataIn - Input Data pointer
 *             Int32U OutSize - Input Data Size
 *
 * Return: None
 *
 * Description: Send and Reads SPI data
 *
 *************************************************************************/
void SPINOR_InOut(Int8U * DataOut, Int32U OutSize,
                  Int8U * DataIn,Int32U InSize)
{
   Int32U Cntr;
   /*Is there a Data to send*/
   if(NULL != DataOut)
   {
      /*while send all data*/
      while(OutSize)
      {
         /*write data into SPI buffer*/
         Cntr = SPI_write_fifo(SLAVE0,DataOut, OutSize);
         OutSize -= Cntr;
         DataOut += Cntr;
      }
   }
   /*wait until SPI si busy*/
   while(SPI_Get_Status() & SPI_BUSY);
   /*If Data should be read*/
   if(NULL != DataIn)
   {
      SPI_rx_fifo_flush();
      /*while receive all data*/
      while(InSize)
      {
         /*Dummy Write*/
         Cntr = SPI_write_fifo(SLAVE0,DataIn,(InSize > SPI_FIFO_SIZE) ? SPI_FIFO_SIZE : InSize);
         /*wait until SPI si busy*/
         while((SPI_Get_Status() & SPI_BUSY));
         /*Read data from fifo*/
         SPI_read_fifo(SLAVE0,DataIn,Cntr);
         InSize -= Cntr;
         DataIn += Cntr;
      }
   }
}

/*************************************************************************
 * Function Name: SPINOR_DP
 * Parameters: None
 *
 * Return: None
 *
 * Description: Put Atmel SPI NOR Flash in Deep Power Down Mode
 *
 *************************************************************************/
void SPINOR_DP(void)
{
   /*Command Deep Power Down*/
   Int8U  DataOut[1] = {CMD_DP};
   /*Drive chip select low*/
   SPI_CS0_LOW;
   /*Send One Byte Command, no
    in data*/
   SPINOR_InOut(DataOut,1,NULL,0);
   /*Drive chip select high*/
   SPI_CS0_HIGH;
}
/*************************************************************************
 * Function Name: SPINOR_Res_DP
 * Parameters: None
 *
 * Return: None
 *
 * Description: Release Atmel SPI NOR Flash from Deep Power Down Mode
 *
 *************************************************************************/
void SPINOR_Res_DP(void)
{
   /*Command Release Deep Power Down*/
   Int8U DataOut[1] = {CMD_RES};
   /*Drive chip select low*/
   SPI_CS0_LOW;
   /*Send One Byte Command, no
    in data*/
   SPINOR_InOut(DataOut,1,NULL,0);
   /*Drive chip select low*/
   SPI_CS0_HIGH;
}
/*************************************************************************
 * Function Name: SPINOR_Get_Device_ID
 * Parameters: Int8U * id - ID pointer
 *
 * Return: None
 *
 * Description: Reads device ID
 *
 *************************************************************************/
void SPINOR_Get_Device_ID(Int8U * id)
{
   /*Command Read ID*/
   Int8U  DataOut[1] = {CMD_RDID};
   /*Drive chip select low*/
   SPI_CS0_LOW;
   /*Send One Byte Command and
   Read 3 Bytes ID*/
   SPINOR_InOut(DataOut,1,id,3);
   /*Drive chip select high*/
   SPI_CS0_HIGH;
}
/*************************************************************************
 * Function Name: SPINOR_Read_Status
 * Parameters: None
 *
 * Return: Int8U Status register
 *
 * Description: Read Atmel SPI NOR Status register
 *
 *************************************************************************/
Int8U SPINOR_Read_Status(void)
{
   /*Command Read Status Register*/
   Int8U DataOut[1] = {CMD_RDSR};
   Int8U Status;
   /*Drive chip select low*/
   SPI_CS0_LOW;
   /*Send One Byte Command, read
   one byte Status*/
   SPINOR_InOut(DataOut,1,&Status,1);
   /*Drive chip select high*/
   SPI_CS0_HIGH;

   return Status;
}
/*************************************************************************
 * Function Name: SPINOR_Erase_Block
 * Parameters: Int32U Page - Page into the Block to be Erased
 *
 * Return: None
 *
 * Description: Erase block (8 pages).
 *
 *************************************************************************/
void SPINOR_Erase_Block(Int32U Page)
{
   Int8U DataOut[5];
   Int32U OutSize;
   /*Command Block Erase*/
   DataOut[0] = CMD_BE;
   /*Form Address Bytes*/
   OutSize = Page2Address(Page,&DataOut[1]) + 1;
   /*Drive chip select low*/
   SPI_CS0_LOW;
   /*Send four or five bytes, no in data*/
   SPINOR_InOut(DataOut,OutSize,NULL,0);
   /*Drive chip select high*/
   SPI_CS0_HIGH;
   /*Wait device to be ready*/
   while(!(SPINOR_Read_Status() & STATUS_RDY));
}

/*************************************************************************
 * Function Name: SPINOR_Read_Page
 * Parameters: Int32U Page - Page Number
 *             Int8U * Dest - Destination buffer
 * Return: None
 *
 * Description: Copy one Page from SPI NOR
 *              into Dest buffer.
 *
 *************************************************************************/
void SPINOR_Read_Page(Int32U Page, Int8U * Dest)
{
   Int8U DataOut[6];
   Int32U OutSize;
   /*Commad Fast Read*/
   DataOut[0] = CMD_FAST_READ;
   /*Form Address Bytes*/
   OutSize = Page2Address(Page,&DataOut[1]) + 1;
   DataOut[OutSize++] = 0xFF;/*Dummy byte*/
   /*Drive chip select low*/
   SPI_CS0_LOW;
   /*Send five or six bytes, read one page*/
   SPINOR_InOut(DataOut,OutSize,Dest,SPINOR_GetPageSize());
   /*Drive chip select high*/
   SPI_CS0_HIGH;
}

/*************************************************************************
 * Function Name: SPINOR_Write_Page
 * Parameters: Int32U Page - Page Number
 *             Int8U * Source - Source buffer
 * Return: None
 *
 * Description: Program one Page into SPI NOR Flash
 *              Read data from Source buffer
 *              Note: The page should be erased before using
 *                    SPINOR_Write_Page funcion
 *************************************************************************/
void SPINOR_Write_Page(Int32U Page, Int8U * Source)
{
   Int8U DataOut[5];
   Int32U OutSize;
   /*Commad write buffer 1*/
   DataOut[0] = CMD_B1W;
   /*Start Address into the Buffer
     is 0*/
   DataOut[1] = 0;
   DataOut[2] = 0;
   DataOut[3] = 0;
   DataOut[4] = 0;
   OutSize = (16384 <= Device->pageNumber) ? 5:4;
   /*Drive chip select low*/
   SPI_CS0_LOW;
   /*Send four or five bytes, no in data*/
   SPINOR_InOut(DataOut,OutSize,NULL,0);
   /*Send one page, no in data*/
   SPINOR_InOut(Source,SPINOR_GetPageSize(),NULL,0);
   /*Drive chip select high*/
   SPI_CS0_HIGH;
   /*Wait device to be ready*/
   while(!(SPINOR_Read_Status() & STATUS_RDY));
   /*Commad Buffer 1 to Main Memory
    Page Program without Built-in Erase*/
   DataOut[0] = CMD_B1MM;
   /*Form Address Bytes*/
   OutSize = Page2Address(Page,&DataOut[1]) + 1;
   /*Drive chip select low*/
   SPI_CS0_LOW;
   /*Send four or five bytes, no in data*/
   SPINOR_InOut(DataOut,OutSize,NULL,0);
   /*Drive chip select high*/
   SPI_CS0_HIGH;
   /*Wait device to be ready*/
   while(!(SPINOR_Read_Status() & STATUS_RDY));
}

/*************************************************************************
 * Function Name: SPINOR_Write_Bytes
 * Parameters: Int32U Page - Page Number
 *             Int32U Addr - Address into Page
 *             Int8U * Source - Source buffer
 *             Int32U Size - Size of the data
 * Return: None
 *
 * Description: Program Bytes (Size) into Page.
 *              Read Data from Source buffer
 *              Note: It will modify only desired bytes. If end of
 *                    page is reached it will  wrap around back to
 *                    the beginning.
 *
 *************************************************************************/
void SPINOR_Write_Bytes(Int32U Page, Int32U Addr, Int8U * Source, Int32U Size)
{
   Int8U DataOut[5];
   Int32U OutSize;
   /*Commad Main Memory to Buffer 1*/
   DataOut[0] = CMD_MMB1;
   /*Form Address Bytes*/
   OutSize = Page2Address(Page,&DataOut[1]) + 1;
   /*Drive chip select low*/
   SPI_CS0_LOW;
   /*Send four of five bytes, no in data*/
   SPINOR_InOut(DataOut,OutSize,NULL,0);
   /*Drive chip select high*/
   SPI_CS0_HIGH;
   /*Wait device to be ready*/
   while(!(SPINOR_Read_Status() & STATUS_RDY));
   /*Commad write buffer 1*/
   DataOut[0] = CMD_B1W;
   /*Start Address into the Buffer*/
   if(16384 <= Device->pageNumber)
   {
      DataOut[1] = ((Addr & 0x0F000000) >> 24);
      DataOut[2] = ((Addr & 0x00FF0000) >> 16);
      DataOut[3] = ((Addr & 0x0000FF00) >> 8);
      DataOut[4] = ((Addr & 0x000000FF) >> 0);
      OutSize = 5;
   }
   else
   {
      DataOut[1] = ((Addr & 0x00FF0000) >> 16);
      DataOut[2] = ((Addr & 0x0000FF00) >> 8);
      DataOut[3] = ((Addr & 0x000000FF) >> 0);
      OutSize = 4;
   }
   /*Drive chip select low*/
   SPI_CS0_LOW;
   /*Send four of five bytes, no in data*/
   SPINOR_InOut(DataOut,OutSize,NULL,0);
   /*Send data bytes, no in data*/
   SPINOR_InOut(Source,Size,NULL,0);
   /*Drive chip select high*/
   SPI_CS0_HIGH;
   /*Wait device to be ready*/
   while(!(SPINOR_Read_Status() & STATUS_RDY));
   /*Commad Buffer 1 to Main Memory
    Page Program with Built-in Erase*/
   DataOut[0] = CMD_B1ME;
   /*Form Address Bytes*/
   OutSize = Page2Address(Page,&DataOut[1]) + 1;
   /*Drive chip select low*/
   SPI_CS0_LOW;
   /*Send four of five bytes, no in data*/
   SPINOR_InOut(DataOut,OutSize,NULL,0);
   /*Drive chip select high*/
   SPI_CS0_HIGH;
   /*Wait device to be ready*/
   while(!(SPINOR_Read_Status() & STATUS_RDY));
}

Int32U SPINOR_GetPageNum(void)
{
   return Device->pageNumber;
}

Int32U SPINOR_GetPageSize(void)
{
   if(Power2Page)
   {
      /*Device is in Power of 2 Page mode.
       Reduce Page Size*/
      return Device->pageSize & ~0xFF;
   }
   return Device->pageSize;
}
/** private functions **/
Int32U Page2Address(Int32U Page, Int8U * dAddr)
{
   Int32U tmp;

   tmp = Page<<(Device->pageOffset - Power2Page);

   if(16384 <= Device->pageNumber)
   {
      *dAddr++ = ((tmp & 0x0F000000) >> 24);
      *dAddr++ = ((tmp & 0x00FF0000) >> 16);
      *dAddr++ = ((tmp & 0x0000FF00) >> 8);
      *dAddr = ((tmp & 0x000000FF) >> 0);

      return 4;
   }

   *dAddr++ = ((tmp & 0x00FF0000) >> 16);
   *dAddr++ = ((tmp & 0x0000FF00) >> 8);
   *dAddr = ((tmp & 0x000000FF) >> 0);

   return 3;
}