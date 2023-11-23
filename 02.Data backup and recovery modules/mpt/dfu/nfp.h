/*********************************************************
 * Module name: nfp.h
 *
 * Copyright 2010, 2011. All Rights Reserved, Crane Chu.
 *
 * The information contained herein is confidential
 * property of Crane Chu. The user, copying, transfer or
 * disclosure of such information is prohibited except
 * by express written agreement with Crane Chu.
 *
 * First written on 2010-01-01 by cranechu@gmail.com
 *
 * Module Description:
 *    Nand Flash Parameters
 *
 *********************************************************/


#ifndef _DFU_NFP_
#define _DFU_NFP_

#include <core\inc\cmn.h>

#pragma pack(1)
typedef struct {
   UINT8    tag[8];
   UINT8    bus_width;
   UINT8    zero0;
   UINT16   page_size_bytes;
   UINT16   page_size_words;
   UINT16   page_per_block;
   UINT32   block_count;
   UINT8    addr_cycle_rw;
   UINT8    addr_cycle_erase;
   UINT8    read_commit_required;
   UINT8    col_cycle;
   UINT8    device_name[40];
   UINT32   nfc_timing1;
   UINT32   nfc_timing2;
   UINT8    ecc_mode;
   UINT8    zero1[3];
   UINT8    padding[176];
   UINT32   crc32;
} NAND_PARAMETERS;

#pragma pack()

#endif


