/*--------------------------------------------------------------------------
 * U S B  -  K e r n e l
 *--------------------------------------------------------------------------
 * Name:    usbcore.h
 * Purpose: USB Core Definitions
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
 * Copyright (c) 2008 Keil - An ARM Crane Chu. All rights reserved.
 * Adaption to LPCxxxx, Copyright (c) 2009 NXP.
 *--------------------------------------------------------------------------*/
 
#ifndef __USBCORE_H__
#define __USBCORE_H__

#include "lpc_usb.h"
#include "usbcfg.h"


/* USB Endpoint Data Structure */
#pragma pack(1)
typedef struct _USB_EP_DATA
{
   UNS_8  *pData;
   UNS_16   Count;
} USB_EP_DATA;
#pragma pack()


/* USB Core Global Variables */
extern UNS_16  USB_DeviceStatus;
extern UNS_8  USB_DeviceAddress;
extern UNS_8  USB_Configuration;
extern UNS_32 USB_EndPointMask;
extern UNS_32 USB_EndPointHalt;
extern UNS_32 USB_EndPointStall;
extern UNS_8  USB_AltSetting[USB_IF_NUM];

/* USB Endpoint 0 Buffer */
extern UNS_8  EP0Buf[USB_MAX_PACKET0];

/* USB Endpoint 0 Data Info */
extern USB_EP_DATA EP0Data;

/* USB Setup Packet */
extern USB_SETUP_PACKET SetupPacket;

/* USB Core Functions */
extern void  USB_ResetCore(void);

#endif  /* __USBCORE_H__ */


