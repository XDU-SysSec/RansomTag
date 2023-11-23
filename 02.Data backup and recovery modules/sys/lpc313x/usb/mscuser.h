/*--------------------------------------------------------------------------
 * U S B  -  K e r n e l
 *--------------------------------------------------------------------------
 * Name:    mscuser.h
 * Purpose: Mass Storage Class Custom User Definitions
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

#include <core\inc\cmn.h>
#include <onfm.h>

#include <lpc_types.h>

#ifndef __MSCUSER_H__
#define __MSCUSER_H__


/* Mass Storage Memory Layout */
/* MSC Disk Image Definitions */
/* Mass Storage Memory Layout */
#define MSC_BlockSize         (SECTOR_SIZE)

/* Max In/Out Packet Size */
#define MSC_FS_MAX_PACKET     (64)
#define MSC_HS_MAX_PACKET     (512)

/* MSC In/Out Endpoint Address */
#define MSC_EP_IN       0x81
#define MSC_EP_OUT      0x01


typedef enum {
   UT_READ,
   UT_WRITE,
   UT_MERGE,
   UT_PREREAD, 
} UT_TYPE;

typedef enum {
   MERGE_NONE,
   MERGE_START,
   MERGE_FINISH,
} MERGE_STAGE;

typedef struct {
   UT_TYPE  type;
   UNS_32   offset;
   UNS_32   length;
   UNS_8*   buffer;
   unsigned char tag;    
} USB_TRANSCATION;

#define UT_LIST_SIZE       (BUFFER_COUNT*2)
extern USB_TRANSCATION  ut_list[UT_LIST_SIZE];
extern volatile UNS_32  ut_pop;
extern volatile UNS_32  ut_push;

extern UNS_32        Read_BulkLen;
extern MERGE_STAGE   merge_stage;

extern void MSC_Init();

/* MSC Requests Callback Functions */
extern UNS_32 MSC_Reset(void);
extern UNS_32 MSC_GetMaxLUN(void);

/* MSC Bulk Callback Functions */
extern void MSC_GetCBW(void);
extern void MSC_SetCSW(void);
extern void MSC_BulkIn(void);
extern void MSC_BulkInNak(void);
extern void MSC_BulkOut(void);
extern void MSC_BulkOutNak(void);

// transfer cbw flag to main.c
extern UNS_8 getCBW_Flag(void);
extern UNS_8 getCBW_LUN(void);
extern UNS_8 getCBW_LUN2(void);

#endif  /* __MSCUSER_H__ */


