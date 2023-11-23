/*--------------------------------------------------------------------------
 * U S B  -  K e r n e l
 *--------------------------------------------------------------------------
 * Name:    usbhw.h
 * Purpose: USB Hardware Function prototypes
 * Version: V1.20
 *--------------------------------------------------------------------------
 * This software is supplied "AS IS" without any warranties, express,
 * implied or statutory, including but not limited to the implied
 * warranties of fitness for purpose, satisfactory quality and
 * noninfringement. Keil extends you a royalty-free right to reproduce
 * and distribute executable files created using this software for use
 * on NXP ARM microcontroller devices only. Nothing else gives
 * you the right to use this software.
 *
 * Copyright (c) 2005-2009 Keil Software.
 * Adaption to LPCxxxx, Copyright (c) 2009 NXP.
 *--------------------------------------------------------------------------*/

#include "includes.h"

#include "lpc_usb.h"

#ifndef __USBHW_H__
#define __USBHW_H__

/* USB Hardware Functions */
extern void  USB_WakeUpCfg(UNS_32  cfg);
extern void  USB_SetAddress(UNS_32 adr);
extern void  USB_Configure(UNS_32  cfg);
extern void  USB_ConfigEP(USB_ENDPOINT_DESCRIPTOR *pEPD);
extern void  USB_DirCtrlEP(UNS_32 dir);
extern void  USB_EnableEP(UNS_32 EPNum);
extern void  USB_DisableEP(UNS_32 EPNum);
extern void  USB_ResetEP(UNS_32 EPNum);
extern void  USB_SetStallEP(UNS_32 EPNum);
extern void  USB_ClrStallEP(UNS_32 EPNum);
extern BOOL_32 USB_SetTestMode(UNS_8 mode);
extern UNS_32 USB_ReadEP(UNS_32 EPNum, UNS_8 *pData);
extern UNS_32 USB_ReadReqEP(UNS_32 EPNum, UNS_8 *pData, UNS_32 len);
extern UNS_32 USB_ReadSetupPkt(UNS_32, UNS_32 *);
extern UNS_32 USB_WriteEP(UNS_32 EPNum, UNS_8 *pData, UNS_32 cnt);


/* TODO: set LED1 to GPIO2, LED2 to GPIO3, so blinking in boot */
#define LED1 (1<<11)
#define LED2 (1<<12)
#define OTG_PWR_EN (1<<13)
#define OTG_OVRCR  (1<<14)

#define LED_SET(led)  {IOCONF_GPIO_M1_SET = led;IOCONF_GPIO_M0_SET  = led;}
#define LED_CLR(led)  {IOCONF_GPIO_M1_SET = led;IOCONF_GPIO_M0_CLR  = led;}
#define LED_TGL(led)  {IOCONF_GPIO_M1_SET = led;IOCONF_GPIO_M0  ^= led;}

#endif  /* __USBHW_H__ */


