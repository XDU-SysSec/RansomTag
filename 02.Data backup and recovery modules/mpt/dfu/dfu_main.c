/*********************************************************
 * Module name: dfu_main.c
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
 *    Create DFU ROM in PC.
 *
 *********************************************************/

#include <core\inc\cmn.h>

#include "bih.h"
#include "nfp.h"
#include "nid.h"

/* Process:
 * - verify the ONFM-FW bin generated
 * - create ONFM-FW image header (OIH)
 * - create nand flash parameters (NFP) table
 * - verify the nand image downloader compiled (NID)
 * - combine ONFM-FW, OIH, NFP, NID as BOOT-IMG
 * - create BOOT-IMG header (BIH)
 * - combine BOOT-IMG and BIH as BOOT-BIN
 * - dd BOOT-BIN to DFU-BIN
 * - unsimgcr DFU-BIN to DFU-ROM
 * - download DFU-ROM with dfu-util or DFUAPP
 * - reboot to start ONFM-FW from NAND. 
 */


#define ONFM_FW_BIN_FILE      "..\\..\\prj\\iar\\onfm_fw.bin"
#define NID_BIN_FILE          "..\\nid\\nid.bin"

IMAGE_HEADER create_img_header()
{
}

NAND_PARAMETERS create_nand_parameters()
{
}

int main()
{
   UINT8* onfm_fw_bin, nid_bin, boot_img;
   UINT32 onfm_fw_len, nid_len, boot_img_len;
   IMAGE_HEADER oih, bih;
   NAND_PARAMETERS nfp = create_nand_parameters();
   FILE* onfm_fw = fopen(ONFM_FW_BIN_FILE, "rb");
   FILE* nid = fopen(NID_BIN_FILE, "rb");
   FILE* boot_bin = fopen("boot.bin", "wb");
   
   if (onfm_fw == NULL)
   {
      printf("ONFM_FW image not found. \n\r");
      return -1;
   }

   if (nid == NULL)
   {
      printf("NID image not found. \n\r");
      return -2;
   }

   fseek(onfm_fw, 0, SEEK_END);
   onfm_fw_len = ftell(onfm_fw);
   onfm_fw_bin = (UINT8*)malloc(onfm_fw_len);
   if (onfm_fw_bin == NULL)
   {
      printf("Memory short. \n\r");
      return -3;
   }
   
   fread(onfm_fw, onfm_fw_bin, onfm_fw_len);

   /* create image header */
   oih = create_img_header(onfm_fw_bin, onfm_fw_len);


   fseek(nid, 0, SEEK_END);
   nid_len = ftell(nid);
   nid_bin = (UINT8*)malloc(nid_len);
   if (nid_bin == NULL)
   {
      printf("Memory short. \n\r");
      return -3;
   }

   fread(nid, nid_bin, nid_len);

   /* combine_image(boot_img, onfm_fw_bin, oih, nfp, nid_bin); */
   boot_img_len = sizeof(IMAGE_HEADER) + 
                  nid_len + 
                  sizeof(NAND_PARAMETERS) +
                  sizeof(IMAGE_HEADER) + 
                  onfm_fw_len;
   boot_img = (UINT8*)malloc(boot_img_len);
   memcpy(&boot_img[sizeof(IMAGE_HEADER)], &nid_bin, nid_len);
   memcpy(&boot_img[sizeof(IMAGE_HEADER)+OFFSET_NAND_PARAM], 
          &nfp, sizeof(NAND_PARAMETERS));
   memcpy(&boot_img[sizeof(IMAGE_HEADER)+OFFSET_ONFM_IMG], 
          &oih, sizeof(IMAGE_HEADER));
   memcpy(&boot_img[sizeof(IMAGE_HEADER)+OFFSET_ONFM_IMG+sizeof(IMAGE_HEADER)], 
          &oih, onfm_fw_len);
   
   /* create image header */
   bih = create_img_header(&boot_img[sizeof(IMAGE_HEADER)], boot_img_len-sizeof(IMAGE_HEADER));

   memcpy(boot_img, &bih, sizeof(IMAGE_HEADER));

   if (boot_bin == NULL)
   {
      printf("NID image not found. \n\r");
      return -2;
   }

   fwrite(boot_bin, boot_img, boot_img_len);
   fclose(boot_bin);

   /* os will close and free all file/mem */
   return 0;
}


