/*********************************************************
 * Module name: nid.h
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
 *    Nand Image Downloader
 *
 *********************************************************/


#ifndef _NID_H_
#define _NID_H_

#include <core\inc\cmn.h>

/* put nand parameter and onfm image on these offset location, and 
 * nand image downloader will get code in this offset after booting
 * through DFU. NO bad block table is used. 
 */
#define OFFSET_NAND_PARAM     (10*1024)   /* 10K code for NID */
#define OFFSET_ONFM_IMG       (OFFSET_NAND_PARAM+PAGE_SIZE)

#endif


