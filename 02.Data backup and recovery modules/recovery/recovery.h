#ifndef _RECOVERY_H_
#define _RECOVERY_H_




#define RANSOMTAG  0x7        //This Irp seems to have come from ransomware
#define RANSOMTAG_MFT  0x8    //This Irp try to modify $MFT
#define RECOVERTAG  0x10
#define MFT_RESERVED_SECTOR_START  850000
#define MFT_RESERVED_SECTOR_END   950032

extern unsigned long pre_write_sector;
extern unsigned long pre_read_sector;

int CheckTag(unsigned char cdb);
int BackupMFT(unsigned long sector_addr, unsigned long sector_count);
int Backup(int ransomtag,unsigned long sector_addr, unsigned long sector_count);






#endif