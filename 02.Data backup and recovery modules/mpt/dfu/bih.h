/*********************************************************
 * Module name: bih.h
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
 *    Boot Image Header
 *
 *********************************************************/


#ifndef _DFU_BIH_
#define _DFU_BIH_

#include <core\inc\cmn.h>

typedef struct {
   UINT32   vector;
   UINT32   magic;
   UINT32   execution_crc32;
   UINT32   padding0[4];
   UINT32   type;
   UINT32   length;
   UINT32   version;
   UINT32   stamp;
   UINT32   zero0;
   UINT32   reserved[15];
   UINT32   header_crc32;
   UINT32   zero1[4];
} IMAGE_HEADER;

#endif

