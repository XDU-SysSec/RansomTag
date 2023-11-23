/*************************************************************************
 *
 *   Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2008
 *
 *    File name   : drv_spinor.h
 *    Description : Atmel SPI NOR Flash driver
 *
 *    History :
 *    1. Date        : 27.3.2009
 *       Author      : Stoyan Choynev
 *       Description : Initial Revison
 *
 *    $Revision: 31493 $
 **************************************************************************/
 
#ifndef __DRV_SPINOR_H
#define __DRV_SPINOR_H

/** include files **/
#include <NXP\iolpc3130.h>
#include "arm_comm.h"

/** definitions **/

/** default settings **/

/** public data **/

/** public functions **/
extern Int32U InitSPINOR();
extern void SPINOR_Res_DP(void);
extern void SPINOR_Get_Device_ID(Int8U *id);
extern void SPINOR_Read_Page(Int32U Page, Int8U * Dest);
extern void SPINOR_Write_Page(Int32U Page, Int8U * Source);
extern void SPINOR_Erase_Block(Int32U Page);
extern void SPINOR_Write_Bytes(Int32U Page, Int32U Addr, Int8U * Source, Int32U Size);
extern Int8U SPINOR_Read_Status(void);
extern Int32U SPINOR_GetPageNum(void);
extern Int32U SPINOR_GetPageSize(void);
#endif /* __DRV_SPINOR_H */
