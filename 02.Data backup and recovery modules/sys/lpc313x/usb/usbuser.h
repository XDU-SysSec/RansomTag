/*--------------------------------------------------------------------------
 * U S B  -  K e r n e l
 *--------------------------------------------------------------------------
 * Name:    usbuser.h
 * Purpose: USB Custom User Definitions
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

#ifndef __USBUSER_H__
#define __USBUSER_H__


/* USB Device Events Callback Functions */
extern void USB_Power_Event(UNS_32  power);
extern void USB_Reset_Event(void);
extern void USB_Suspend_Event(void);
extern void USB_Resume_Event(void);
extern void USB_WakeUp_Event(void);
extern void USB_SOF_Event(void);
extern void USB_Error_Event(UNS_32 error);

/* USB Endpoint Callback Events */
#define USB_EVT_SETUP       1   /* Setup Packet */
#define USB_EVT_OUT         2   /* OUT Packet */
#define USB_EVT_IN          3   /*  IN Packet */
#define USB_EVT_OUT_NAK     4   /* OUT Packet - Not Acknowledged */
#define USB_EVT_IN_NAK      5   /*  IN Packet - Not Acknowledged */
#define USB_EVT_OUT_STALL   6   /* OUT Packet - Stalled */
#define USB_EVT_IN_STALL    7   /*  IN Packet - Stalled */
#define USB_EVT_OUT_DMA_EOT 8   /* DMA OUT EP - End of Transfer */
#define USB_EVT_IN_DMA_EOT  9   /* DMA  IN EP - End of Transfer */
#define USB_EVT_OUT_DMA_NDR 10  /* DMA OUT EP - New Descriptor Request */
#define USB_EVT_IN_DMA_NDR  11  /* DMA  IN EP - New Descriptor Request */
#define USB_EVT_OUT_DMA_ERR 12  /* DMA OUT EP - Error */
#define USB_EVT_IN_DMA_ERR  13  /* DMA  IN EP - Error */

#define PAYLOAD_HEADER_SIZE (0x0C)


/* USB Endpoint Events Callback Pointers */
extern void (* const USB_P_EP[4])(UNS_32 event);

/* USB Endpoint Events Callback Functions */
extern void USB_EndPoint0(UNS_32 event);
extern void USB_EndPoint1(UNS_32 event);
extern void USB_EndPoint2(UNS_32 event);
extern void USB_EndPoint3(UNS_32 event);

/* USB Core Events Callback Functions */
extern void USB_Configure_Event(void);
extern void USB_Interface_Event(void);
extern void USB_Feature_Event(void);

#endif  /* __USBUSER_H__ */


