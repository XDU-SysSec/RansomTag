/*********************************************************
 * Module name: buf.h
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
 *    Buffer writing sectors in RAM (e.g. FIFO), until
 *    enough sectors to write as an MPP (multiple plane
 *    page), which can program parallelly. Also force to
 *    flush when stop or non-seqential writing happened.
 *
 *********************************************************/


#ifndef _FTL_RAMBUFFER_H_
#define _FTL_RAMBUFFER_H_


/*********************************************************
 * Funcion Name: BUF_Init
 *
 * Description:
 *    Init the variables and data in ram write_buffer.
 *
 * Return Value:
 *    N/A
 *
 * Parameter List:
 *    N/A
 *
 * NOTES:
 *    N/A
 *
 *********************************************************/
void BUF_Init();


/*********************************************************
 * Funcion Name: BUF_PutSector
 *
 * Description:
 *    Put one sector to rambuffer.
 *
 * Return Value:
 *    N/A
 *
 * Parameter List:
 *    addr     IN    the logical address of the sector
 *    buffer   IN    the buffer holding the sector data
 *
 * NOTES:
 *    N/A
 *
 *********************************************************/
void BUF_PutSector(LSADDR addr, void* buffer);


/*********************************************************
 * Funcion Name: BUF_GetPage
 *
 * Description:
 *    Get a page from buffer management.
 *
 * Return Value:
 *    N/A
 *
 * Parameter List:
 *    addr     IN    the logical address of the sector
 *    buffer   IN    the buffer to return
 *
 * NOTES:
 *    N/A
 *
 *********************************************************/
void BUF_GetPage(PGADDR* addr, void** buffer,Boolean flag,Boolean mft);


/*********************************************************
 * Funcion Name: BUF_Free
 *
 * Description:
 *    Release a buffer.
 *
 * Return Value:
 *    N/A
 *
 * Parameter List:
 *    buffer   IN    the buffer to release
 *
 * NOTES:
 *    N/A
 *
 *********************************************************/
void BUF_Free(void* buffer);


/*********************************************************
 * Funcion Name: BUF_Allocate
 *
 * Description:
 *    Allocate a buffer.
 *
 * Return Value:
 *    N/A
 *
 * Parameter List:
 *    N/A
 *
 * NOTES:
 *    N/A
 *
 *********************************************************/
void* BUF_Allocate();

#endif


