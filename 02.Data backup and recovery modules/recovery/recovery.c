
#include <core\inc\cmn.h>
#include <onfm.h>
#include "recovery.h"



int CheckTag(unsigned char cdb)
{
    if (cdb == 0)
    {
        return 0; //There is no tag
    }

    if ((cdb & RANSOMTAG) == RANSOMTAG)
    {
        //PRINTF("This write request has ransomtag and does not aim on $MFT!\n");
        return 1; //ransomtag
    }
    
    if ((cdb & RANSOMTAG_MFT) == RANSOMTAG_MFT)
    {
        //PRINTF("This write request aims on $MFT! cdb & RANSOMTAG_MFT=%d\n",cdb & RANSOMTAG_MFT);
        return 2; //MFT tag
    }

    if ((cdb & RECOVERTAG) == RECOVERTAG)
    {
        //PRINTF("This request is recovery program!\n");
        return 3; //MFT tag
    }
    
    return -1;
}

int Backup(int ransomtag, unsigned long sector_addr, unsigned long sector_count)
{
    
    

    switch (ransomtag)
    {
    case -1:
        //PRINTF("Can not recognize the tag!\n");
        break;
    case 0:
        //PRINTF("There is no tag!\n");
        break;
    case 1:
        //reserve_block()
        break;
    case 2:
        BackupMFT(sector_addr, sector_count);
        //reserve_block()
        break;
    default:
        break;
    }
    
    return 0;
}

int BackupMFT(unsigned long sector_addr, unsigned long sector_count)
{
    unsigned char MFT_Buffer[4096]={0};
    unsigned char *PMFT_Buffer = MFT_Buffer;
    int ret = 0;
    
    if (sector_count>8)
    {
      //PRINTF("MFT_Buffer overflowed! sector_count=%u\n",sector_count);
      return -1;
    }
    
    ret = ONFM_Read(sector_addr, sector_count, PMFT_Buffer,FALSE,FALSE);

    if (ret != 0)
    {
        //PRINTF("Read MFT failed!\n");
        return -1;
    }
    
    if (MFT_Reserved_currentsec < MFT_RESERVED_SECTOR_END - sector_count)
    {
        ret = ONFM_Write(MFT_Reserved_currentsec, sector_count, PMFT_Buffer,FALSE,FALSE);
        if (ret == 0)
        {
            //PRINTF("MFT entry have been backed up!\n");
            MFT_Reserved_currentsec += sector_count;
            return 0;
        }
        else
        {
            //PRINTF("Backup MFT failed!\n");
            return -1;
        }
    }
    else
    {
        //PRINTF("There is no enough space for MFT reserving!\n");
        return -1;
    }
    
    
}