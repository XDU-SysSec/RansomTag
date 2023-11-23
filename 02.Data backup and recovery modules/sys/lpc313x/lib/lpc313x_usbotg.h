/***********************************************************************
 * $Id:: lpc313x_usbotg.h 3557 2010-05-20 00:31:30Z usb10131           $
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

#ifndef LPC313X_USBOTG_H
#define LPC313X_USBOTG_H

#include "lpc_types.h"
#include "lpc313x_chip.h"

#ifdef __cplusplus
extern "C"
{
#endif


/***********************************************************************
* USB Register Structures
**********************************************************************/

/* USB Module Register Structure */
typedef volatile struct
{
   /*Capability registers*/
   volatile UNS_8  caplength;     /*Capability register length*/
   volatile UNS_8  _otg1[1];
   volatile UNS_16 hciversion;    /*Host interface version number*/
   volatile UNS_32 hcsparams;     /*Host controller structural parameters*/
   volatile UNS_32 hccparams;     /*Host controller capability parameters*/
   volatile UNS_32 _otg2[5];      /*10Ch-11Fh 20 Reserved */
   volatile UNS_16 dciversion;    /*Device interface version number*/
   volatile UNS_16 _otg3[1];
   volatile UNS_32 dccparams;     /*Device controller capability parameters*/
   volatile UNS_32 _otg4a[4];     /*128h-134h 16 Reserved */
   volatile UNS_32 usb_up_int;    /*USB interrupt test mode*/
   volatile UNS_32 _otg4b[1];     /*13Ch 4 Reserved */

   /* Operational registers */
   volatile UNS_32 usbcmd;        /*USB command*/
   volatile UNS_32 usbsts;        /*USB status*/
   volatile UNS_32 usbintr;       /*USB interrupt enable*/
   volatile UNS_32 frindex;       /*USB frame index*/
   volatile UNS_32 _otg5[1];
   volatile UNS_32 periodiclistbase__deviceaddr;    /*Frame list base address__USB device address*/
   volatile UNS_32 asynclistaddr__endpointlistaddr; /*Next asynchronous list address__Address of endpoint list in memory*/
   volatile UNS_32 ttctrl;        /*Asynchronous buffer status for embedded TT*/
   volatile UNS_32 burstsize;     /*Programmable burst size*/
   volatile UNS_32 txfilltuning;  /*Host transmit pre-buffer packet tuning*/
   volatile UNS_32 txttfilltuning;/*Host TT transmit pre-buffer packet tuning*/
   volatile UNS_32 _otg6[1];
   volatile UNS_32 _otg7[2];      /*170h-174h 8 Reserved n/a*/
   volatile UNS_32 endptnak;
   volatile UNS_32 endptnaken;
   volatile UNS_32 configflag;    /*Configured flag register*/
   volatile UNS_32 portsc1;       /*Port status/control 1*/
   volatile UNS_32 _otg8[7];      /*190h-1A3h 20 Reserved n/a*/
   volatile UNS_32 otgsc;         /*OTG status and control*/
   volatile UNS_32 usbmode;       /*USB device mode*/
   volatile UNS_32 endptsetupstat;/*Endpoint setup status*/
   volatile UNS_32 endptprime;    /*Endpoint initialization*/
   volatile UNS_32 endptflush;    /*Endpoint de-initialization*/
   volatile UNS_32 endptstatus;   /*Endpoint status*/
   volatile UNS_32 endptcomplete; /*Endpoint complete*/
   volatile UNS_32 endptctrl[4];    /*Endpoint control 0 - 3*/

} USB_OTG_REGS_T;

#if 1
#ifndef __REG
#define __REG(n)	*((volatile UNS_32*)(n))
#endif
//__REG (USBOTG_BASE + (n))
#define USB_REG(n)            *((volatile UNS_32*)(USBOTG_BASE | (n)))
#define USB_DEV_ID            USB_REG(0x000)
#define USB_DEV_HWGENERAL     USB_REG(0x004)
#define USB_DEV_HWHOST        USB_REG(0x008)
#define USB_DEV_HWDEVICE      USB_REG(0x00C)
#define USB_DEV_HWTXBUF       USB_REG(0x010)
#define USB_DEV_HWRXBUF       USB_REG(0x014)
#define USB_DEV_CAPLENGTH     USB_REG(0x100)
#define USB_DEV_HCSPARAMS     USB_REG(0x104)
#define USB_DEV_DCIVERSION    USB_REG(0x120)
#define USB_DEV_DCCPARAMS     USB_REG(0x124)
#define USB_DEV_USBCMD        USB_REG(0x140)
#define USB_DEV_USBSTS        USB_REG(0x144)
#define USB_DEV_USBINTR       USB_REG(0x148)
#define USB_DEV_FRINDEX       USB_REG(0x14C)
#define USB_DEV_DEVADDR       USB_REG(0x154)
#define USB_DEV_EPLISTADDR    USB_REG(0x158)
#define USB_DEV_BURSTSIZE     USB_REG(0x160)
#define USB_DEV_PORTSC1       USB_REG(0x184)
#define USB_DEV_USBMODE       USB_REG(0x1A8)
#define USB_DEV_EPSETUPSTAT   USB_REG(0x1AC)
#define USB_DEV_EPPRIME       USB_REG(0x1B0)
#define USB_DEV_EPFLUSH       USB_REG(0x1B4)
#define USB_DEV_EPSTAT        USB_REG(0x1B8)
#define USB_DEV_EPCOMPLETE    USB_REG(0x1BC)
#define USB_DEV_EPCTRL0       USB_REG(0x1C0)
#define USB_DEV_EPCTRL1       USB_REG(0x1C4)
#define USB_DEV_EPCTRL2       USB_REG(0x1C8)
#define USB_DEV_EPCTRL3       USB_REG(0x1CC)
#define USB_DEV_EPCTRL4       USB_REG(0x1D0)
#define USB_DEV_EPCTRLX(LEp)  USB_REG(0x1C0 + ((LEp) << 2) )
#define USB_DEV_EPNAK         USB_REG(0x178)
#define USB_DEV_EPNAKE        USB_REG(0x17C)
#endif

/* dTD Transfer Description */
typedef volatile struct
{
   volatile UNS_32 next_dTD;
   volatile UNS_32 total_bytes ;
   volatile UNS_32 buffer0;
   volatile UNS_32 buffer1;
   volatile UNS_32 buffer2;
   volatile UNS_32 buffer3;
   volatile UNS_32 buffer4;
   volatile UNS_32 reserved;
}  DTD_T;

/* dQH  Queue Head */
typedef volatile struct
{
   volatile UNS_32 cap;
   volatile UNS_32 curr_dTD;
   volatile UNS_32 next_dTD;
   volatile UNS_32 total_bytes;
   volatile UNS_32 buffer0;
   volatile UNS_32 buffer1;
   volatile UNS_32 buffer2;
   volatile UNS_32 buffer3;
   volatile UNS_32 buffer4;
   volatile UNS_32 reserved;
   volatile UNS_32 setup[2];
   volatile UNS_32 gap[4];
}  DQH_T;


/* bit defines for USBCMD register */
#define USBCMD_RS	    _BIT(0)
#define USBCMD_RST	  _BIT(1)
#define USBCMD_ATDTW 	_BIT(12)
#define USBCMD_SUTW	  _BIT(13)

/* bit defines for USBSTS register */
#define USBSTS_UI	  _BIT(0)
#define USBSTS_UEI	_BIT(1)
#define USBSTS_PCI	_BIT(2)
#define USBSTS_URI	_BIT(6)
#define USBSTS_SRI	_BIT(7)
#define USBSTS_SLI	_BIT(8)
#define USBSTS_NAKI	_BIT(16)

/* bit defines for DEVICEADDR register */
#define USBDEV_ADDR_AD	_BIT(24)
#define USBDEV_ADDR(n)	_SBF(25, ((n) & 0x7F))

/* bit defines for PRTSC1 register */
#define USBPRTS_CCS   _BIT(0)
#define USBPRTS_PE    _BIT(2)
#define USBPRTS_FPR   _BIT(6)
#define USBPRTS_SUSP	_BIT(7)
#define USBPRTS_PR    _BIT(8)
#define USBPRTS_HSP   _BIT(9)
#define USBPRTS_PLPSCD  _BIT(23)
#define USBPRTS_PFSC    _BIT(24)

/* bit defines for USBMODE register */
#define USBMODE_CM_IDLE	_SBF(0, 0x0)
#define USBMODE_CM_DEV	_SBF(0, 0x2)
#define USBMODE_CM_HOST	_SBF(0, 0x3)
#define USBMODE_SLOM    _BIT(3)
#define USBMODE_SDIS    _BIT(4)

/* bit defines for EP registers*/
#define USB_EP_BITPOS(n) (((n) & 0x80)? (0x10 | ((n) & 0x7)) : ((n) & 0x7))

/* bit defines EPcontrol registers*/
#define EPCTRL_RXS	  _BIT(0)
#define EPCTRL_RX_TYPE(n) _SBF(2,((n) & 0x3))
#define EPCTRL_RX_CTL	  _SBF(2,0)
#define EPCTRL_RX_ISO	  _SBF(2,1)
#define EPCTRL_RX_BLK	  _SBF(2,2)
#define EPCTRL_RXI	  _BIT(5)
#define EPCTRL_RXR	  _BIT(6)
#define EPCTRL_RXE	  _BIT(7)
#define EPCTRL_TXS	  _BIT(16)
#define EPCTRL_TX_TYPE(n) _SBF(18,((n) & 0x3))
#define EPCTRL_TX_CTL	  _SBF(18,0)
#define EPCTRL_TX_ISO	  _SBF(18,1)
#define EPCTRL_TX_BLK	  _SBF(18,2)
#define EPCTRL_TX_INT	  _SBF(18,3)
#define EPCTRL_TXI	  _BIT(21)
#define EPCTRL_TXR	  _BIT(22)
#define EPCTRL_TXE	  _BIT(23)

/* dQH field and bit defines */
/* Temp fixed on max, should be taken out of table */
#define QH_MAX_CTRL_PAYLOAD       0x03ff
#define QH_MAX_PKT_LEN_POS            16
#define QH_MAXP(n)                _SBF(16,((n) & 0x3FF))
#define QH_IOS                    _BIT(15)
#define QH_ZLT                    _BIT(29)

/* dTD field and bit defines */
#define TD_NEXT_TERMINATE         _BIT(0)
#define TD_IOC                    _BIT(15)

/* Total physical enpoints*/
#define EP_NUM_MAX	8

/**********************************************************************
 * Macro to access USB_OTG registers
 **********************************************************************/
#define USB_OTG   ((USB_OTG_REGS_T*)(USBOTG_BASE + 0x100))

#ifdef __cplusplus
}
#endif

#endif /* LPC313X_USBOTG_H */

