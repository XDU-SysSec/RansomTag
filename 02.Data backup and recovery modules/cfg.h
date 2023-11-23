/*********************************************************
 * Module name: cfg.h
 *
 * Copyright 2010, 2011. All Rights Reserved, Crane Chu.
 *
 * This file is part of OpenNFM.
 *
 * OpenNFM is free software: you can redistribute it and/or 
 * modify it under the terms of the GNU General Public 
 * License as published by the Free Software Foundation, 
 * either version 3 of the License, or (at your option) any 
 * later version.
 * 
 * OpenNFM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied 
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
 * PURPOSE. See the GNU General Public License for more 
 * details.
 *
 * You should have received a copy of the GNU General Public 
 * License along with OpenNFM. If not, see 
 * <http://www.gnu.org/licenses/>.
 *
 * First written on 2010-01-01 by cranechu@gmail.com
 *
 * Module Description:
 *    Compile Time Configuration of NAND, bus and etc.
 *    User need to modify this file according to HW platform.
 *
 *********************************************************/


#ifndef _ONFM_CFG_H_
#define _ONFM_CFG_H_


/* develop configurable parameter */

/* reclaim for static wear leveling when difference of two block is larger
 * than this threshold.
 */
#define STATIC_WL_THRESHOLD         (1000)

/* reserve some block for bad block replacement */
#define GOOD_BLOCK_PERCENT          (98)
#define OVER_PROVISION_RATE         (3)
/* more pmt cache would decrease WA */
#define PMT_CACHE_COUNT             (4)

/* choose different nand configuration */
#define  SIM_NAND             (0)
#define  K9WAG08              (1)
#define  K9MBG08              (2)
#define  K9F4G08U0M           (3)

#define CFG_NAND_TYPE         (K9WAG08)

#if (CFG_NAND_TYPE == K9WAG08)
#define SECTOR_SIZE_SHIFT           (9)   /* fixed */
#define SECTOR_PER_PAGE_SHIFT       (2)   /* 2, 3, 4 */
#define PAGE_PER_BLOCK_SHIFT        (6)   /* <=8 */
#define BLOCK_PER_PLANE_SHIFT       (11)  /* >=7 */
#define PLANE_PER_DIE_SHIFT         (1)   /* 0 or 1 */
#define DIE_PER_CHIP_SHIFT          (0)   /* 0 or 1 */
#define CHIP_COUNT_SHIFT            (0)   /* any 0*/

#define CFG_NAND_COL_CYCLE          (2)
#define CFG_NAND_ROW_CYCLE          (3)

#endif


#if (CFG_NAND_TYPE == K9MBG08)
#define SECTOR_SIZE_SHIFT           (9)   /* fixed */
#define SECTOR_PER_PAGE_SHIFT       (2)   /* 2, 3, 4 */
#define PAGE_PER_BLOCK_SHIFT        (7)   /* <=8 */
#define BLOCK_PER_PLANE_SHIFT       (10)  /* >=7 */
#define PLANE_PER_DIE_SHIFT         (1)   /* 0 or 1 */
#define DIE_PER_CHIP_SHIFT          (1)   /* 0 or 1 */
#define CHIP_COUNT_SHIFT            (0)   /* TODO: 2 */

#define CFG_NAND_COL_CYCLE          (2)
#define CFG_NAND_ROW_CYCLE          (3)

#endif


#if (CFG_NAND_TYPE == SIM_NAND)
#define SECTOR_SIZE_SHIFT           (9)   /* fixed */
#define SECTOR_PER_PAGE_SHIFT       (2)   /* 2, 3, 4 */
#define PAGE_PER_BLOCK_SHIFT        (4)   /* <=8 */
#define BLOCK_PER_PLANE_SHIFT       (7)  /* >=7 */
#define PLANE_PER_DIE_SHIFT         (0)   /* 0 or 1 */
#define DIE_PER_CHIP_SHIFT          (1)   /* 0 or 1 */
#define CHIP_COUNT_SHIFT            (2)   /* any */

#define CFG_NAND_COL_CYCLE          (2)
#define CFG_NAND_ROW_CYCLE          (4)

#endif

#if(CFG_NAND_TYPE == K9F4G08U0M)

#define SECTOR_SIZE_SHIFT           (9)   /* fixed  (512)*/  
#define SECTOR_PER_PAGE_SHIFT       (2)   /* 2, 3, 4 */
#define PAGE_PER_BLOCK_SHIFT        (6)   /* <=8  (64) */  
#define BLOCK_PER_PLANE_SHIFT       (11)  /* >=7 */
#define PLANE_PER_DIE_SHIFT         (1)   /* 0 or 1 */
#define DIE_PER_CHIP_SHIFT          (0)   /* 0 or 1 */
#define CHIP_COUNT_SHIFT            (0)   /* any */

#define CFG_NAND_COL_CYCLE          (2)
#define CFG_NAND_ROW_CYCLE          (3)

#endif

#endif


