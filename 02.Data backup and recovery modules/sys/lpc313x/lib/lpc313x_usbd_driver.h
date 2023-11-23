/***********************************************************************
 * $Id:: lpc313x_usbd_driver.h 2382 2009-10-27 22:12:38Z pdurgesh      $
 *
 * Project: LPC313X USB-OTG controller definitions
 *
 * Description:
 *     This file contains the structure definitions and manifest
 *     constants for the LPC313X chip family component:
 *         Universal Serial Bus - On The Go (USB-OTG)
 *
 ***********************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
 **********************************************************************/

#ifndef __USBUSER_H__
#define __USBUSER_H__


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

/* call back structure */
typedef struct _USB_INIT_
{
   UNS_32 ep0_maxp;
   /* USB Device Events Callback Functions */
   void (* USB_Power_Event)(UNS_32  power);
   void (* USB_Reset_Event)(void);
   void (* USB_Suspend_Event)(void);
   void (* USB_Resume_Event)(void);
   void (* USB_WakeUp_Event)(void);
   void (* USB_SOF_Event)(void);
   void (* USB_Error_Event)(UNS_32 error);
   /* USB Core Events Callback Functions */
   void (* USB_Configure_Event)(void);
   void (* USB_Interface_Event)(void);
   void (* USB_Feature_Event)(void);

   /* USB Endpoint Events Callback Pointers */
   void (* USB_P_EP[4])(UNS_32 event);

} LPC_USBDRV_INIT_T;

/* Driver functions */
void USB_Init(LPC_USBDRV_INIT_T* cbs);
void USB_Connect(BOOL_32 con);
void USB_ISR(void);

#endif  /* __USBUSER_H__ */
