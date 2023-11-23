/*--------------------------------------------------------------------------
 * U S B  -  K e r n e l
 *--------------------------------------------------------------------------
 * Name:    usb.h
 * Purpose: USB Definitions
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
 
#include "lpc_types.h"

#ifndef __USB_H__
#define __USB_H__

#if defined(__GNUC__)
/* As per http://gcc.gnu.org/onlinedocs/gcc/Attribute-Syntax.html#Attribute-Syntax,
6.29 Attributes Syntax
"An attribute specifier list may appear as part of a struct, union or
enum specifier. It may go either immediately after the struct, union
or enum keyword, or after the closing brace. The former syntax is
preferred. Where attribute specifiers follow the closing brace, they
are considered to relate to the structure, union or enumerated type
defined, not to any enclosing declaration the type specifier appears
in, and the type defined is not complete until after the attribute
specifiers."
So use POST_PACK immediately after struct keyword
*/
#define PRE_PACK
#define POST_PACK	__attribute__((__packed__))
#define ALIGNED(n)  __attribute__((aligned (n)))
#elif defined(__arm)
#define PRE_PACK	__packed
#define POST_PACK
#define ALIGNED(n)  __align(n)
#elif defined(__ICCARM__)
/* hard code these keyword */
#endif



#pragma pack(1)
struct _WB_T
{
   UNS_8 L;
   UNS_8 H;
};
typedef struct _WB_T WB_T;

union __WORD_BYTE
{
   UNS_16 W;
   WB_T WB;
} ;
typedef union __WORD_BYTE WORD_BYTE;

#pragma pack()


/* bmRequestType.Dir */
#define REQUEST_HOST_TO_DEVICE     0
#define REQUEST_DEVICE_TO_HOST     1

/* bmRequestType.Type */
#define REQUEST_STANDARD           0
#define REQUEST_CLASS              1
#define REQUEST_VENDOR             2
#define REQUEST_RESERVED           3

/* bmRequestType.Recipient */
#define REQUEST_TO_DEVICE          0
#define REQUEST_TO_INTERFACE       1
#define REQUEST_TO_ENDPOINT        2
#define REQUEST_TO_OTHER           3

#pragma pack(1)
struct _BM_T
{
   UNS_8 Recipient : 5;
   UNS_8 Type      : 2;
   UNS_8 Dir       : 1;
};
typedef struct _BM_T BM_T;

/* bmRequestType Definition */
union _REQUEST_TYPE
{
   UNS_8 B;
   BM_T BM;
} ;
typedef union _REQUEST_TYPE REQUEST_TYPE;

#pragma pack()

/* USB Standard Request Codes */
#define USB_REQUEST_GET_STATUS                 0
#define USB_REQUEST_CLEAR_FEATURE              1
#define USB_REQUEST_SET_FEATURE                3
#define USB_REQUEST_SET_ADDRESS                5
#define USB_REQUEST_GET_DESCRIPTOR             6
#define USB_REQUEST_SET_DESCRIPTOR             7
#define USB_REQUEST_GET_CONFIGURATION          8
#define USB_REQUEST_SET_CONFIGURATION          9
#define USB_REQUEST_GET_INTERFACE              10
#define USB_REQUEST_SET_INTERFACE              11
#define USB_REQUEST_SYNC_FRAME                 12

/* USB GET_STATUS Bit Values */
#define USB_GETSTATUS_SELF_POWERED             0x01
#define USB_GETSTATUS_REMOTE_WAKEUP            0x02
#define USB_GETSTATUS_ENDPOINT_STALL           0x01

/* USB Standard Feature selectors */
#define USB_FEATURE_ENDPOINT_STALL             0
#define USB_FEATURE_REMOTE_WAKEUP              1
#define USB_FEATURE_TEST_MODE                  2

/* USB Default Control Pipe Setup Packet */
#pragma pack(1)
struct _USB_SETUP_PACKET
{
   REQUEST_TYPE bmRequestType;
   UNS_8         bRequest;
   WORD_BYTE  wValue;
   WORD_BYTE  wIndex;
   UNS_16        wLength;
} ;
typedef struct _USB_SETUP_PACKET USB_SETUP_PACKET;

#pragma pack()


/* USB Descriptor Types */
#define USB_DEVICE_DESCRIPTOR_TYPE             1
#define USB_CONFIGURATION_DESCRIPTOR_TYPE      2
#define USB_STRING_DESCRIPTOR_TYPE             3
#define USB_INTERFACE_DESCRIPTOR_TYPE          4
#define USB_ENDPOINT_DESCRIPTOR_TYPE           5
#define USB_DEVICE_QUALIFIER_DESCRIPTOR_TYPE   6
#define USB_OTHER_SPEED_CONFIG_DESCRIPTOR_TYPE 7
#define USB_INTERFACE_POWER_DESCRIPTOR_TYPE    8
#define USB_OTG_DESCRIPTOR_TYPE                     9
#define USB_DEBUG_DESCRIPTOR_TYPE                  10
#define USB_INTERFACE_ASSOCIATION_DESCRIPTOR_TYPE  11

/* USB Device Classes */
#define USB_DEVICE_CLASS_RESERVED              0x00
#define USB_DEVICE_CLASS_AUDIO                 0x01
#define USB_DEVICE_CLASS_COMMUNICATIONS        0x02
#define USB_DEVICE_CLASS_HUMAN_INTERFACE       0x03
#define USB_DEVICE_CLASS_MONITOR               0x04
#define USB_DEVICE_CLASS_PHYSICAL_INTERFACE    0x05
#define USB_DEVICE_CLASS_POWER                 0x06
#define USB_DEVICE_CLASS_PRINTER               0x07
#define USB_DEVICE_CLASS_STORAGE               0x08
#define USB_DEVICE_CLASS_HUB                   0x09
#define USB_DEVICE_CLASS_MISCELLANEOUS         0xEF
#define USB_DEVICE_CLASS_VENDOR_SPECIFIC       0xFF

/* bmAttributes in Configuration Descriptor */
#define USB_CONFIG_POWERED_MASK                0x40
#define USB_CONFIG_BUS_POWERED                 0x80
#define USB_CONFIG_SELF_POWERED                0xC0
#define USB_CONFIG_REMOTE_WAKEUP               0x20

/* bMaxPower in Configuration Descriptor */
#define USB_CONFIG_POWER_MA(mA)                ((mA)/2)

/* bEndpointAddress in Endpoint Descriptor */
#define USB_ENDPOINT_DIRECTION_MASK            0x80
#define USB_ENDPOINT_OUT(addr)                 ((addr) | 0x00)
#define USB_ENDPOINT_IN(addr)                  ((addr) | 0x80)

/* bmAttributes in Endpoint Descriptor */
#define USB_ENDPOINT_TYPE_MASK                 0x03
#define USB_ENDPOINT_TYPE_CONTROL              0x00
#define USB_ENDPOINT_TYPE_ISOCHRONOUS          0x01
#define USB_ENDPOINT_TYPE_BULK                 0x02
#define USB_ENDPOINT_TYPE_INTERRUPT            0x03
#define USB_ENDPOINT_SYNC_MASK                 0x0C
#define USB_ENDPOINT_SYNC_NO_SYNCHRONIZATION   0x00
#define USB_ENDPOINT_SYNC_ASYNCHRONOUS         0x04
#define USB_ENDPOINT_SYNC_ADAPTIVE             0x08
#define USB_ENDPOINT_SYNC_SYNCHRONOUS          0x0C
#define USB_ENDPOINT_USAGE_MASK                0x30
#define USB_ENDPOINT_USAGE_DATA                0x00
#define USB_ENDPOINT_USAGE_FEEDBACK            0x10
#define USB_ENDPOINT_USAGE_IMPLICIT_FEEDBACK   0x20
#define USB_ENDPOINT_USAGE_RESERVED            0x30

/* USB Standard Device Descriptor 设备描述符 */
#pragma pack(1)
struct _USB_DEVICE_DESCRIPTOR
{
   UNS_8  bLength;
   UNS_8  bDescriptorType;
   UNS_16  bcdUSB;
   UNS_8  bDeviceClass;
   UNS_8  bDeviceSubClass;
   UNS_8  bDeviceProtocol;
   UNS_8  bMaxPacketSize0;
   UNS_16  idVendor;
   UNS_16  idProduct;
   UNS_16  bcdDevice;
   UNS_8  iManufacturer;
   UNS_8  iProduct;
   UNS_8  iSerialNumber;
   UNS_8  bNumConfigurations;
} ;
typedef struct _USB_DEVICE_DESCRIPTOR USB_DEVICE_DESCRIPTOR;

/* USB 2.0 Device Qualifier Descriptor */
struct _USB_DEVICE_QUALIFIER_DESCRIPTOR
{
   UNS_8  bLength;
   UNS_8  bDescriptorType;
   UNS_16  bcdUSB;
   UNS_8  bDeviceClass;
   UNS_8  bDeviceSubClass;
   UNS_8  bDeviceProtocol;
   UNS_8  bMaxPacketSize0;
   UNS_8  bNumConfigurations;
   UNS_8  bReserved;
} ;
typedef struct _USB_DEVICE_QUALIFIER_DESCRIPTOR USB_DEVICE_QUALIFIER_DESCRIPTOR;

/* USB Standard Configuration Descriptor 配置描述符 */
struct _USB_CONFIGURATION_DESCRIPTOR
{
   UNS_8  bLength; //9
   UNS_8  bDescriptorType; //2
   UNS_16  wTotalLength;// 包含所有描述子的长度(配置、接口、端点、类相关、厂商相关)
   UNS_8  bNumInterfaces; //该配置包含几个接口
   UNS_8  bConfigurationValue; //选择该配置时，传递给setConfiguration()的参数
   UNS_8  iConfiguration; //在字符串描述符中的序号
   UNS_8  bmAttributes; //D7:保留位1 D6:是否由总线供电 D5:是否支持远程唤醒 D4-D0：保留为0
   UNS_8  bMaxPower; //单位2mA,该设备在该CONF下全速工作时需要从总线获取的电流(以2mA为单位)
} ;
typedef struct _USB_CONFIGURATION_DESCRIPTOR USB_CONFIGURATION_DESCRIPTOR;

/* USB Standard Interface Descriptor 接口描述符 */
struct _USB_INTERFACE_DESCRIPTOR
{
   UNS_8  bLength; // 长度：9
   UNS_8  bDescriptorType; // 4
   UNS_8  bInterfaceNumber; //该接口的序号：在该配置所支持的众多接口中的序号，从0开始
   UNS_8  bAlternateSetting; // 用来选择该接口的某个setting
   UNS_8  bNumEndpoints; // 该接口支持多少个非0的EP
   UNS_8  bInterfaceClass; 
   UNS_8  bInterfaceSubClass;
   UNS_8  bInterfaceProtocol;
   UNS_8  iInterface; //index 在众多字符串描述符中的序号
} ;
typedef struct _USB_INTERFACE_DESCRIPTOR USB_INTERFACE_DESCRIPTOR;

/* USB Standard Endpoint Descriptor 端点描述符*/
struct _USB_ENDPOINT_DESCRIPTOR
{
   UNS_8  bLength; // 描述符长度，一般7bytes
   UNS_8  bDescriptorType; // 描述符类型 5
   UNS_8  bEndpointAddress; // 端点的特性： Bit7:方向（0-out;1-in。in与out是对host来说的) Bit6-4:保留为0 Bit3-0:端点号
   UNS_8  bmAttributes;//端点的类型 Bit1-0：传输类型(CTRL,ISO,BULK,INTR) Bit3-2同步类型 Bit5-4 使用类型(数据EP，反馈EP...)
   UNS_16 wMaxPacketSize; // 端点最大包长
   UNS_8  bInterval; // 主机查询EP的间隔，以帧或微帧为单位
} ;
typedef struct _USB_ENDPOINT_DESCRIPTOR USB_ENDPOINT_DESCRIPTOR;

/* USB String Descriptor 字符串描述符*/
struct _USB_STRING_DESCRIPTOR
{
   UNS_8  bLength;
   UNS_8  bDescriptorType; // 3
   UNS_16  bString/*[]*/; //描述符描述的内容
}  ;
typedef struct _USB_STRING_DESCRIPTOR USB_STRING_DESCRIPTOR;

/* USB Common Descriptor */
struct _USB_COMMON_DESCRIPTOR
{
   UNS_8  bLength;
   UNS_8  bDescriptorType;
} ;
typedef struct _USB_COMMON_DESCRIPTOR USB_COMMON_DESCRIPTOR;

/* USB Other Speed Configuration */
struct _USB_OTHER_SPEED_CONFIGURATION
{
   UNS_8  bLength;
   UNS_8  bDescriptorType;
   UNS_16 wTotalLength;
   UNS_8  bNumInterfaces;
   UNS_8  bConfigurationValue;
   UNS_8  IConfiguration;
   UNS_8  bmAttributes;
   UNS_8  bMaxPower;
} ;
typedef struct _USB_OTHER_SPEED_CONFIGURATION USB_OTHER_SPEED_CONFIGURATION;

#pragma pack()

#endif  /* __USB_H__ */
