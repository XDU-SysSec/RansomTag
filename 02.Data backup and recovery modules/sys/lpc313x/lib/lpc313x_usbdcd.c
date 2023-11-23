/***********************************************************************
 * $Id:: lpc313x_usbdcd.c 3557 2010-05-20 00:31:30Z usb10131           $
 *
 * Project: LPC31xx USB device controller driver
 *
 * Description:
 *     This file contains driver support for the LPC31xx USB device
 *  controller driver.
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
 *********************************************************************/

#include <string.h>
#include "lpc_usb.h"
#include "lpc313x_usbotg.h"
#include "lpc313x_sysreg.h"
#include "lpc313x_evt_router.h"
#include "lpc313x_cgu_driver.h"
#include "lpc313x_usbd_driver.h"


#pragma data_alignment=2048
DQH_T ep_QH[EP_NUM_MAX];

#pragma data_alignment=32
DTD_T ep_TD[EP_NUM_MAX];

static UNS_32 ep_read_len[EP_NUM_MAX/2];
volatile UNS_32 DevStatusFS2HS = FALSE;
static LPC_USBDRV_INIT_T g_drv;

/*
*  Get Endpoint Physical Address
*    Parameters:      EPNum: Endpoint Number
*                       EPNum.0..3: Address
*                       EPNum.7:    Dir
*    Return Value:    Endpoint Physical Address
*/

static UNS_32 EPAdr(UNS_32 EPNum)
{
   UNS_32 val;

   val = (EPNum & 0x0F) << 1; // 取三位地址
   if (EPNum & 0x80) // bit8为1时，地址值+1
   {
      val += 1;
   }
   return (val);
}


/*
*  USB Connect Function
*   Called by the User to Connect/Disconnect USB
*    Parameters:      con:   Connect/Disconnect
*    Return Value:    None
*/

void USB_Connect(BOOL_32 con)
{
   if (con)
      USB_OTG->usbcmd |= USBCMD_RS;
   else
      USB_OTG->usbcmd &= ~USBCMD_RS;
}


/*
*  USB Reset Function
*   Called automatically on USB Reset
*    Return Value:    None
*/

void USB_Reset(void)
{
   UNS_32 i;

   /* force high speed */
   DevStatusFS2HS = TRUE;  //FALSE;

   /* disable all EPs */
   for (i = 1; i < EP_NUM_MAX / 2; i++)
   {
     USB_OTG->endptctrl[i] &= ~(EPCTRL_RXE | EPCTRL_TXE);
   }

   /* Clear all pending interrupts */
   USB_OTG->endptnak   = 0xFFFFFFFF;
   USB_OTG->endptnaken = 0;
   USB_OTG->usbsts     = 0xFFFFFFFF;
   USB_OTG->endptsetupstat = USB_OTG->endptsetupstat;
   USB_OTG->endptcomplete  = USB_OTG->endptcomplete;
   while (USB_OTG->endptprime)                  /* Wait until all bits are 0 */
   {
   }
   USB_OTG->endptflush = 0xFFFFFFFF;
   while (USB_OTG->endptflush); /* Wait until all bits are 0 */


   /* Set the interrupt Threshold control interval to 0 */
   USB_OTG->usbcmd &= ~0x00FF0000;

   /* Zero out the Endpoint queue heads */
   memset((void*)ep_QH, 0, EP_NUM_MAX * sizeof(DQH_T));
   /* Zero out the device transfer descriptors */
   memset((void*)ep_TD, 0, EP_NUM_MAX * sizeof(DTD_T));
   memset((void*)ep_read_len, 0, sizeof(ep_read_len));

   /* Configure the Endpoint List Address */
   /* make sure it in on 64 byte boundary !!! */
   /* init list address */
   USB_OTG->asynclistaddr__endpointlistaddr = (UNS_32)ep_QH;
   /* Initialize device queue heads for non ISO endpoint only */
   for (i = 0; i < EP_NUM_MAX; i++)
   {
      ep_QH[i].next_dTD = (UNS_32) & ep_TD[i];
   }
   /* Enable interrupts */
   USB_OTG->usbintr =  USBSTS_UI
                       | USBSTS_UEI
                       | USBSTS_PCI
                       | USBSTS_URI
                       | USBSTS_SLI
                       | USBSTS_NAKI;

   /* enable ep0 IN and ep0 OUT */
   ep_QH[0].cap  = QH_MAXP(g_drv.ep0_maxp)
                   | QH_IOS
                   | QH_ZLT;
   ep_QH[1].cap  = QH_MAXP(g_drv.ep0_maxp)
                   | QH_IOS
                   | QH_ZLT;

   /* enable EP0 */
   USB_OTG->endptctrl[0] = EPCTRL_RXE | EPCTRL_RXR | EPCTRL_TXE | EPCTRL_TXR;

}



/*
*  USB Remote Wakeup Function
*   Called automatically on USB Remote Wakeup
*    Return Value:    None
*/

void USB_WakeUp(void)
{

   //if (USB_DeviceStatus & USB_GETSTATUS_REMOTE_WAKEUP)
   {
      /* Set FPR bit in PORTSCX reg p63 */
      USB_OTG->portsc1 |= USBPRTS_FPR ;
   }
}


/*
*  USB Remote Wakeup Configuration Function
*    Parameters:      cfg:   Enable/Disable
*    Return Value:    None
*/

void USB_WakeUpCfg(UNS_32 cfg)
{
   /* Not needed */
}


/*
*  USB Set Address Function
*    Parameters:      adr:   USB Address
*    Return Value:    None
*/

void USB_SetAddress(UNS_32 adr)
{
   USB_OTG->periodiclistbase__deviceaddr = USBDEV_ADDR(adr);
   USB_OTG->periodiclistbase__deviceaddr |= USBDEV_ADDR_AD;
}


/*
*  USB set test mode Function
*    Parameters:      mode:   test mode
*    Return Value:    TRUE if supported else FALSE
*/

BOOL_32 USB_SetTestMode(UNS_8 mode)
{
   UNS_32 portsc;
   if ((mode > 0) && (mode < 8))
   {
      portsc = USB_OTG->portsc1 & ~(0xF << 16);

      USB_OTG->portsc1 = portsc | (mode << 16);
      return TRUE;
   }
   return (FALSE);
}

/*
*  USB Configure Function
*    Parameters:      cfg:   Configure/Deconfigure
*    Return Value:    None
*/

void USB_Configure(UNS_32 cfg)
{

}

/*
*  Configure USB Endpoint according to Descriptor
*    Parameters:      pEPD:  Pointer to Endpoint Descriptor
*    Return Value:    None
*/

void USB_ConfigEP(USB_ENDPOINT_DESCRIPTOR *pEPD)
{
   UNS_32 num, lep;
   UNS_32 ep_cfg;
   UNS_8  bmAttributes;

   lep = pEPD->bEndpointAddress & 0x7F;
   num = EPAdr(pEPD->bEndpointAddress);

   ep_cfg = USB_OTG->endptctrl[lep];
   /* mask the attributes we are not-intersetd in */
   bmAttributes = pEPD->bmAttributes & USB_ENDPOINT_TYPE_MASK;
   /* set EP type */
   if (bmAttributes != USB_ENDPOINT_TYPE_ISOCHRONOUS)
   {
      /* init EP capabilities */
      ep_QH[num].cap  = QH_MAXP(pEPD->wMaxPacketSize)
                        | QH_IOS
                        | QH_ZLT ;

      /* The next DTD pointer is INVALID */
      ep_TD[num].next_dTD = 0x01 ;
   }
   else
   {
      /* init EP capabilities */
      ep_QH[num].cap  = QH_MAXP(0x400) | QH_ZLT ;
      //tmudsDcdIP9028_IsoPreparedTD ( ) ;
      //tmudsDcdIP9028_IsoPrime ( ) ;
   }
   /* setup EP control register */
   if (pEPD->bEndpointAddress & 0x80)
   {
      ep_cfg &= ~0xFFFF0000;
      ep_cfg |= EPCTRL_TX_TYPE(bmAttributes)
                | EPCTRL_TXR;
   }
   else
   {
      ep_cfg &= ~0xFFFF;
      ep_cfg |= EPCTRL_RX_TYPE(bmAttributes)
                | EPCTRL_RXR;
   }
   USB_OTG->endptctrl[lep] = ep_cfg;
}


/*
*  Set Direction for USB Control Endpoint
*    Parameters:      dir:   Out (dir == 0), In (dir <> 0)
*    Return Value:    None
*/

void USB_DirCtrlEP(UNS_32 dir)
{
   /* Not needed */
}


/*
*  Enable USB Endpoint
*    Parameters:      EPNum: Endpoint Number
*                       EPNum.0..3: Address
*                       EPNum.7:    Dir
*    Return Value:    None
*/

void USB_EnableEP(UNS_32 EPNum)
{
   UNS_32 lep, bitpos;

   lep = EPNum & 0x0F;

   if (EPNum & 0x80)
   {
      USB_OTG->endptctrl[lep] |= EPCTRL_TXE;
   }
   else
   {
      USB_OTG->endptctrl[lep] |= EPCTRL_RXE;
   }

   /* enable NAK interrupt */
   bitpos = USB_EP_BITPOS(EPNum);
   USB_OTG->endptnaken |= _BIT(bitpos);
}


/*
*  Disable USB Endpoint
*    Parameters:      EPNum: Endpoint Number
*                       EPNum.0..3: Address
*                       EPNum.7:    Dir
*    Return Value:    None
*/

void USB_DisableEP(UNS_32 EPNum)
{
   UNS_32 lep, bitpos;

   lep = EPNum & 0x0F;

   if (EPNum & 0x80)
   {
      USB_OTG->endptctrl[lep] &= ~EPCTRL_TXE;
   }
   else
   {
      /* disable NAK interrupt */
      bitpos = USB_EP_BITPOS(EPNum);
      USB_OTG->endptnaken &= ~_BIT(bitpos);
      USB_OTG->endptctrl[lep] &= ~EPCTRL_RXE;
   }
}


/*
*  Reset USB Endpoint
*    Parameters:      EPNum: Endpoint Number
*                       EPNum.0..3: Address
*                       EPNum.7:    Dir
*    Return Value:    None
*/

void USB_ResetEP(UNS_32 EPNum)
{
   UNS_32 bit_pos = USB_EP_BITPOS(EPNum);
   UNS_32 lep = EPNum & 0x0F;

   /* flush EP buffers */
   USB_OTG->endptflush = _BIT(bit_pos);
   while (USB_OTG->endptflush & _BIT(bit_pos));
   /* reset data toggles */
   if (EPNum & 0x80)
   {
      USB_OTG->endptctrl[lep] |= EPCTRL_TXR;
   }
   else
   {
      USB_OTG->endptctrl[lep] |= EPCTRL_RXR;
   }
}


/*
*  Checks if endpoint is enabled
*    Parameters:      EPNum: Endpoint Number
*                       EPNum.0..3: Address
*                       EPNum.7:    Dir
*    Return Value:    1 for enabled
*/
UNS_32 IsEPEnabled(UNS_32 EPNum)
{
   UNS_32 lep = EPNum & 0x0F;
   if (EPNum & 0x80)
   {
      if (USB_DEV_EPCTRLX(lep) & EPCTRL_TXE)
         return 1;
      else
         return 0;
   }
   else
   {
      if (USB_DEV_EPCTRLX(lep) & EPCTRL_RXE)
         return 1;
      else
         return 0;
   }
}


/*
*  Set Stall for USB Endpoint
*    Parameters:      EPNum: Endpoint Number
*                       EPNum.0..3: Address
*                       EPNum.7:    Dir
*    Return Value:    None
*/

void USB_SetStallEP(UNS_32 EPNum)
{
   UNS_32 lep;

   lep = EPNum & 0x0F;

   if (EPNum & 0x80)
   {
      USB_OTG->endptctrl[lep] |= EPCTRL_TXS;
   }
   else
   {
      USB_OTG->endptctrl[lep] |= EPCTRL_RXS;
   }
}


/*
*  Clear Stall for USB Endpoint
*    Parameters:      EPNum: Endpoint Number
*                       EPNum.0..3: Address
*                       EPNum.7:    Dir
*    Return Value:    None
*/

void USB_ClrStallEP(UNS_32 EPNum)
{
   UNS_32 lep;

   lep = EPNum & 0x0F;

   if (EPNum & 0x80)
   {
      USB_OTG->endptctrl[lep] &= ~EPCTRL_TXS;
      /* reset data toggle */
      USB_OTG->endptctrl[lep] |= EPCTRL_TXR;
   }
   else
   {
      USB_OTG->endptctrl[lep] &= ~EPCTRL_RXS;
      /* reset data toggle */
      USB_OTG->endptctrl[lep] |= EPCTRL_RXR;
   }
}

void USB_ProgDTD(UNS_32 Edpt, UNS_32 ptrBuff, UNS_32 TsfSize)
{
   DTD_T*  pDTD ;

   pDTD    = (DTD_T*) & ep_TD[ Edpt ];

   /* Zero out the device transfer descriptors */
   memset((void*)pDTD, 0, sizeof(DTD_T));
   /* The next DTD pointer is INVALID */
   pDTD->next_dTD = 0x01 ;

   /* Length */
   pDTD->total_bytes = ((TsfSize & 0x7fff) << 16);
   pDTD->total_bytes |= TD_IOC ;
   pDTD->total_bytes |= 0x80 ;

   pDTD->buffer0 = ptrBuff;
   pDTD->buffer1 = (ptrBuff + 0x1000) & 0xfffff000;
   pDTD->buffer2 = (ptrBuff + 0x2000) & 0xfffff000;
   pDTD->buffer3 = (ptrBuff + 0x3000) & 0xfffff000;
   pDTD->buffer4 = (ptrBuff + 0x4000) & 0xfffff000;


   ep_QH[Edpt].next_dTD = (UNS_32)(&ep_TD[ Edpt ]);
   ep_QH[Edpt].total_bytes &= (~0xC0) ;
}
/*
*  Read USB Endpoint Data
*    Parameters:      EPNum: Endpoint Number
*                       EPNum.0..3: Address
*                       EPNum.7:    Dir
*                     pData: Pointer to Data Buffer
*    Return Value:    Number of bytes read
*/
UNS_32 USB_ReadSetupPkt(UNS_32 EPNum, UNS_32 *pData)
{
   UNS_32  setup_int, cnt = 0;
   UNS_32 num = EPAdr(EPNum);

   setup_int = USB_OTG->endptsetupstat ;
   /* Clear the setup interrupt */
   USB_OTG->endptsetupstat = setup_int;
   /* ********************************** */
   /*  Check if we have received a setup */
   /* ********************************** */

   if (setup_int & _BIT(0))                    /* Check only for bit 0 */
      /* No setup are admitted on other endpoints than 0 */
   {
      do
      {
         /* Setup in a setup - must considere only the second setup */
         /*- Set the tripwire */
         USB_OTG->usbcmd |= USBCMD_SUTW ;

         /* Transfer Set-up data to the gtmudsCore_Request buffer */
         pData[0] = ep_QH[num].setup[0];
         pData[1] = ep_QH[num].setup[1];
         cnt = 8;

      }
      while (!(USB_OTG->usbcmd & USBCMD_SUTW)) ;

      /* setup in a setup - Clear the tripwire */
      USB_OTG->usbcmd &= (~USBCMD_SUTW);
   }
   while ((setup_int = USB_OTG->endptsetupstat) != 0)
   {
      /* Clear the setup interrupt */
      USB_OTG->endptsetupstat = setup_int;
   }

   return cnt;
}

/*
*  Enque read request
*    Parameters:      EPNum: Endpoint Number
*                       EPNum.0..3: Address
*                       EPNum.7:    Dir
*                     pData: Pointer to Data Buffer
*    Return Value:    Number of bytes read
*/

UNS_32 USB_ReadReqEP(UNS_32 EPNum, UNS_8 *pData, UNS_32 len)
{
   UNS_32 num = EPAdr(EPNum);
   UNS_32 n = USB_EP_BITPOS(EPNum);

   USB_ProgDTD(num, (UNS_32)pData, len);
   ep_read_len[EPNum & 0x0F] = len;

   /* prime the endpoint for read */
   USB_OTG->endptprime |= _BIT(n) ;

   /* check if priming succeeded */
   while (USB_OTG->endptprime & _BIT(n));

   return len;
}

/*
*  Read USB Endpoint Data
*    Parameters:      EPNum: Endpoint Number
*                       EPNum.0..3: Address
*                       EPNum.7:    Dir
*                     pData: Pointer to Data Buffer
*    Return Value:    Number of bytes read
*/

UNS_32 USB_ReadEP(UNS_32 EPNum, UNS_8 *pData)
{
   UNS_32 cnt, n;
   DTD_T*  pDTD ;

   n = EPAdr(EPNum); // 端点逻辑地址转换成物理地址，映射表在手册中有
   pDTD    = (DTD_T*) & ep_TD [ n ] ; //取第N个dTD

   /* return the total bytes read */
   cnt  = (pDTD->total_bytes >> 16) & 0x7FFF; //   取高16位，再取其中的0-15位 30:16位是total bytes
   cnt = ep_read_len[EPNum & 0x0F] - cnt; // 取ep逻辑地址的len - 字节长度

   return (cnt);
}


/*
*  Write USB Endpoint Data
*    Parameters:      EPNum: Endpoint Number
*                       EPNum.0..3: Address
*                       EPNum.7:    Dir
*                     pData: Pointer to Data Buffer
*                     cnt:   Number of bytes to write
*    Return Value:    Number of bytes written
*/

UNS_32 USB_WriteEP(UNS_32 EPNum, UNS_8 *pData, UNS_32 cnt)
{
   UNS_32 n = USB_EP_BITPOS(EPNum);

   USB_ProgDTD(EPAdr(EPNum), (UNS_32)pData, cnt);

   /* prime the endpoint for transmit */
   USB_OTG->endptprime |= _BIT(n) ;

   /* check if priming succeeded */
   while (USB_OTG->endptprime & _BIT(n));

   return (cnt);
}

/*
*  USB Initialize Function
*   Called by the User to initialize USB
*    Return Value:    None
*/

void USB_Init(LPC_USBDRV_INIT_T* cbs)
{
   /* enable USB PLL */
   UNS_32 bank = EVT_GET_BANK(EVT_usb_atx_pll_lock);
   UNS_32 bit_pos = EVT_usb_atx_pll_lock & 0x1F;

   /* copy callbacks */
   memcpy(&g_drv, cbs, sizeof(LPC_USBDRV_INIT_T));
   /*maxp for EP0 should be atleast 8 */
   if (g_drv.ep0_maxp == 0)
      g_drv.ep0_maxp = 64;

   /* enable USB to AHB clock */
   cgu_clk_en_dis(CGU_SB_USB_OTG_AHB_CLK_ID, TRUE);
   /* enable clock to Event router */
   cgu_clk_en_dis(CGU_SB_EVENT_ROUTER_PCLK_ID, TRUE);

   /* reset USB block */
   cgu_soft_reset_module(USB_OTG_AHB_RST_N_SOFT);

   /* enable USB OTG PLL */
   SYS_REGS->usb_atx_pll_pd_reg = 0x0;
   /* wait for PLL to lock */
   while (!(EVTR_REGS->rsr[bank] & _BIT(bit_pos)));

   /* enable USB AHB clock */
   cgu_clk_en_dis(CGU_SB_USB_OTG_AHB_CLK_ID, TRUE);

   /* reset the controller */
   USB_OTG->usbcmd = USBCMD_RST;
   /* wait for reset to complete */
   while (USB_OTG->usbcmd & USBCMD_RST);

   /* Program the controller to be the USB device controller */
   USB_OTG->usbmode =   USBMODE_CM_DEV
                        /*| USBMODE_SDIS*/
                        | USBMODE_SLOM ;

   /* set OTG transcever in proper state */
   USB_OTG->otgsc = _BIT(3) | _BIT(0);

   /* large burst cycle */
   USB_OTG->burstsize = 0x4040;

   USB_Reset();
   USB_SetAddress(0);
}


/*
*  USB Interrupt Service Routine
*/

void USB_ISR(void)
{
   UNS_32 disr, val, n;

   disr = USB_OTG->usbsts;                      /* Device Interrupt Status */
   USB_OTG->usbsts = disr;

   /* Device Status Interrupt (Reset, Connect change, Suspend/Resume) */
   if (disr & USBSTS_URI)                      /* Reset */
   {
      USB_Reset();
      if (g_drv.USB_Reset_Event)
         g_drv.USB_Reset_Event();
      goto isr_end;
   }

   if (disr & USBSTS_SLI)                   /* Suspend */
   {
      USB_OTG->otgsc &= ~_BIT(0);
      if (g_drv.USB_Suspend_Event)
         g_drv.USB_Suspend_Event();
   }

   if (disr & USBSTS_PCI)                  /* Resume */
   {
      /* check if device isoperating in HS mode or full speed */
      if (USB_OTG->portsc1 & _BIT(9))
         DevStatusFS2HS = TRUE;

      USB_OTG->otgsc |= _BIT(0);

      if (g_drv.USB_Resume_Event)
         g_drv.USB_Resume_Event();
   }

   /* handle setup status interrupts */
   val = USB_OTG->endptsetupstat;
   /* Only EP0 will have setup packets so call EP0 handler */
   if (val)
   {
      /* Clear the endpoint complete CTRL OUT & IN when */
      /* a Setup is received */
      USB_OTG->endptcomplete = 0x00010001;
      /* enable NAK inetrrupts */
      USB_OTG->endptnaken |= 0x00010001;
      if (g_drv.USB_P_EP[0])
         g_drv.USB_P_EP[0](USB_EVT_SETUP);
   }

   /* handle completion interrupts */
   val = USB_OTG->endptcomplete;
   if (val)
   {
      USB_OTG->endptnak = val;
      for (n = 0; n < EP_NUM_MAX / 2; n++)
      {
         if (val & _BIT(n))
         {
            if (g_drv.USB_P_EP[n])
               g_drv.USB_P_EP[n](USB_EVT_OUT);

            USB_OTG->endptcomplete = _BIT(n);
         }
         if (val & _BIT(n + 16))
         {
            ep_TD [(n << 1) + 1 ].total_bytes &= 0xC0;
            if (g_drv.USB_P_EP[n])
               g_drv.USB_P_EP[n](USB_EVT_IN);

            USB_OTG->endptcomplete = _BIT(n + 16);
         }
      }
   }

   if (disr & USBSTS_NAKI)
   {
      val = USB_OTG->endptnak;
      val &= USB_OTG->endptnaken;
      /* handle NAK interrupts */
      if (val)
      {
         for (n = 0; n < EP_NUM_MAX / 2; n++)
         {

            if (val & _BIT(n))
            {
               if (g_drv.USB_P_EP[n])
                  g_drv.USB_P_EP[n](USB_EVT_OUT_NAK);
            }
            if (val & _BIT(n + 16))
            {
               if (g_drv.USB_P_EP[n])
                  g_drv.USB_P_EP[n](USB_EVT_IN_NAK);
            }
         }

         USB_OTG->endptnak = val;
      }
   }

   /* Start of Frame Interrupt */
   if (disr & USBSTS_SRI)
   {
      if (g_drv.USB_SOF_Event)
         g_drv.USB_SOF_Event();
   }

   /* Error Interrupt */
   if (disr & USBSTS_UEI)
   {
      if (g_drv.USB_Error_Event)
         g_drv.USB_Error_Event(disr);
   }

isr_end:
   return;
}
