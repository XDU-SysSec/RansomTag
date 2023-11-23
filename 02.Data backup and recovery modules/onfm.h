/*********************************************************
 * Module name: onfm.h
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
 *    Header file of ONFM. User of ONFM only need to 
 *    include this one file. 
 *
 *********************************************************/


#ifndef _ONFM_HEADER_H_
#define _ONFM_HEADER_H_

int ONFM_Format();

int ONFM_Capacity();

int ONFM_Mount();

int ONFM_Read(unsigned long   sector_addr,
              unsigned long   sector_count,
              void*           sector_data,
              Boolean flag,
              Boolean mft);

int ONFM_Write(unsigned long  sector_addr,
               unsigned long  sector_count,
               void*          sector_data,
               Boolean flag,
               Boolean mft);

int ONFM_Unmount();


//ransomtag

extern unsigned long MFT_Reserved_currentsec;   //保留扇区的起始扇区号

#endif


