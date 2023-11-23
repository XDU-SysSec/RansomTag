#ifndef _RANSOMTAG_H
#define _RANSOMTAG_H

#include <share/vmm_types.h>
#include <storage.h>
#include <common/list.h>


/*************************  Common Start  ************************************************************************************************/
#define RANSOMTAG  0x7         //This Irp seems to have come from ransomware
#define RANSOMTAG_MFT  0x8    //This Irp try to modify $MFT
#define RECOVERTAG  0x10
#define BITMAP_LBA 2072

typedef unsigned long long int linux_virtaddr;
typedef unsigned long long int Win_virtaddr;
typedef unsigned long long int linux_list_head;
#ifndef _RANSOMTAG_H
#define _RANSOMTAG_H

#include <share/vmm_types.h>
#include <storage.h>
#include <common/list.h>


/*************************  Common Start  ************************************************************************************************/
#define RANSOMTAG  0x7         //This Irp seems to have come from ransomware
#define RANSOMTAG_MFT  0x8    //This Irp try to modify $MFT
#define RECOVERTAG  0x10
#define BITMAP_LBA 2072

typedef unsigned long long int linux_virtaddr;
typedef unsigned long long int Win_virtaddr;
typedef unsigned long long int linux_list_head;

struct cbw {
	u32 dCBWSignature;
	u32 dCBWTag;
	u32 dCBWDataTransferLength;
	u8  bmCBWFlags;
	u8  bCBWLUN;			/* includes 4 reserved bits */
	u8  bCBWCBLength;		/* includes 3 reserved bits */
	u8 CBWCB[16];
};
typedef struct cbw_info
{
    struct cbw cbw;
    unsigned char direction;    // 0 is out, 1 is in
    u32 lba;
    u16 n_blocks;
} cbw_info;

void SetBitmap(int i);
void ClearBitmap(int i);
int TestBitmap(int i);
u32 reversebytes_uint32t(u32 value);
u64 reversebytes_uint64t(u64 value);
unsigned char *mem_find_str(unsigned char *full_data, int full_data_len, unsigned char *substr);


//qtd
int is_CBW(struct ehci_qtd_meta *qtdm, struct cbw_info *cbw_info);
void parse_CBW(struct cbw_info *cbw_info);
struct ehci_qtd *get_qtd_from_qtdm(struct ehci_qtd_meta *qtdm);
// qtd-end

//calculate entropy start
extern long long byte_count[256];
double __attribute__((target("sse"))) my_log(double a);
double __attribute__((target("sse"))) _entropy_calc(long long length);
int __attribute__((target("sse"))) entropy_calc(void);
int buffer_byte_count(u8 *data, long long  length);

// calculate entropy end
/*************************  Common End  ************************************************************************************************/




/*************************  Linux Begin  ***************************************************************************************************/
#define LINUX_ADDR_SIZE 8
#define LINUX_LISTHEAD_SIZE 16
#define SG_CHAIN 0x01UL
#define SG_END		0x02UL

#define INIT_TASK 0xffffffff82612780       //address in System.map
#define START_INIT_TASK 0xFFFFFFFF82600000    //address in System.map
#define CURRENT_TASK_GS_OFFSET 0x16BC0     //CURRENT_TASK offset in System.map
#define THIS_CPU_OFF  0x10358   //this_cpu_off in System.map

#define USER_ADDR_SPACE_START 0x0000000000000000
#define USER_ADDR_SPACE_END 0x00007FFFFFFFFFFF

#define KERNEL_ADDR_SPACE_START 0xFFFF800000000000
#define KERNEL_ADDR_SPACE_END 0xFFFFFFFFFFFFFFFF

//struct task_struct fields offset
#define TASK_STRUCT_COMM_OFFSET      0xA58     //task name
#define TASK_STRUCT_STACK_OFFSET     0x18      //task kernel stack
#define TASK_STRUCT_TASKS_OFFSET     0x7A0     //children tasks
#define TASK_STRUCT_PARENT_OFFSET    0x8B8     //parent
#define TASK_STRUCT_PID_OFFSET       0x8A0     //pid
#define TASK_STRUCT_FILES_OFFSET     0xAA0     //files
#define TASK_STRUCT_MM_OFFSET        0x7F0     //mm
#define TASK_STRUCT_ACTIVE_MM_OFFSET        0x7F8     //active_mm


//struct mm_struct fields offset
#define MM_STRUCT_PGD_OFFSET         0x50      //mm_struct->pgd

//struct files_struct fields offset
#define FILES_STRUCT_FDT_OFFSET      0x20      //files->fdt

//struct fdtable fields offset
#define FDTABLE_FD_OFFSET           0x8       //fdtable->fd
#define NR_OPEN_DEFAULT              64

//struct file fields offset
#define SIZEOF_STRUCT_FILE           256
#define STRUCT_FILE_FPATH_OFFSET     0x10      //file->f_path
#define STRUCT_FILE_FINODE_OFFSET    0x20      //file->f_inode
#define STRUCT_FILE_FMAPPING_OFFSET  0xF0      //file->f_mapping

//ext4 related
#define EXT4_SUPERBLOCK_LBA		     0	//ext volume starts with 2048 offset of the disk
#define EXT4_SUPERBLOCK_OFFSET_BYTE  1024	// SuperBlock starts on (block0 + 1024Bytes)
#define EXT4_SUPERBLOCK_SIZE		 1024
#define EXT4_GROUP_DESC_OFFSET_BYTE  4096	// SuperBlock + 4096 Bytes (8 sectors) = Group Descriptor[0]		     
#define EXT4_GROUP_DESC_SIZE         32


extern unsigned int linux_need_tag;
extern bool read_ext4_metadata;
struct scatterlist
{
	linux_virtaddr	page_link;
	unsigned int	offset;
	unsigned int	length;
	linux_virtaddr	dma_address;
};

struct linux_list_head {
    linux_virtaddr list_head_field;
    linux_virtaddr next;
    linux_virtaddr prev;
};

typedef struct linux_task_info
{
    linux_virtaddr task_struct;
    linux_virtaddr stack;
    linux_virtaddr files;
    linux_virtaddr fdtable;
    struct linux_list_head list_head_tasks;
    u32 pid;
    linux_virtaddr parent_task_struct;
    u32 parent_pid;
    linux_virtaddr mm;
    linux_virtaddr mm_pgd;
    u64 mm_pgd_phys;
    linux_virtaddr active_mm;
    linux_virtaddr active_mm_pgd;
    u64 active_mm_pgd_phys;
    char task_name[32];

} linux_task_info, *linux_task_info_ptr;

/*
 * Constants relative to the data blocks
 */
#define	EXT4_NDIR_BLOCKS		12
#define	EXT4_IND_BLOCK			EXT4_NDIR_BLOCKS
#define	EXT4_DIND_BLOCK			(EXT4_IND_BLOCK + 1)
#define	EXT4_TIND_BLOCK			(EXT4_DIND_BLOCK + 1)
#define	EXT4_N_BLOCKS			(EXT4_TIND_BLOCK + 1)

/*
 * Structure of a blocks group descriptor
 */
struct ext4_group_desc
{
	u32	bg_block_bitmap_lo;	/* Blocks bitmap block */
	u32	bg_inode_bitmap_lo;	/* Inodes bitmap block */
	u32	bg_inode_table_lo;	/* Inodes table block */
	u16	bg_free_blocks_count_lo;/* Free blocks count */
	u16	bg_free_inodes_count_lo;/* Free inodes count */
	u16	bg_used_dirs_count_lo;	/* Directories count */
	u16	bg_flags;		/* EXT4_BG_flags (INODE_UNINIT, etc) */
	u32  bg_exclude_bitmap_lo;   /* Exclude bitmap for snapshots */
	u16  bg_block_bitmap_csum_lo;/* crc32c(s_uuid+grp_num+bbitmap) LE */
	u16  bg_inode_bitmap_csum_lo;/* crc32c(s_uuid+grp_num+ibitmap) LE */
	u16  bg_itable_unused_lo;	/* Unused inodes count */
	u16  bg_checksum;		/* crc16(sb_uuid+group+desc) */
	//for 64bit address
	//u32	bg_block_bitmap_hi;	/* Blocks bitmap block MSB */
	//u32	bg_inode_bitmap_hi;	/* Inodes bitmap block MSB */
	//u32	bg_inode_table_hi;	/* Inodes table block MSB */
	//u16	bg_free_blocks_count_hi;/* Free blocks count MSB */
	//u16	bg_free_inodes_count_hi;/* Free inodes count MSB */
	//u16	bg_used_dirs_count_hi;	/* Directories count MSB */
	//u16  bg_itable_unused_hi;    /* Unused inodes count MSB */
	//u32  bg_exclude_bitmap_hi;   /* Exclude bitmap block MSB */
	//u16  bg_block_bitmap_csum_hi;/* crc32c(s_uuid+grp_num+bbitmap) BE */
	//u16  bg_inode_bitmap_csum_hi;/* crc32c(s_uuid+grp_num+ibitmap) BE */
	//u32   bg_reserved;
};


/*
 * Structure of an inode on the disk
 */
struct ext4_inode {
	u16	i_mode;		/* File mode */
	u16	i_uid;		/* Low 16 bits of Owner Uid */
	u32	i_size_lo;	/* Size in bytes */
	u32	i_atime;	/* Access time */
	u32	i_ctime;	/* Inode Change time */
	u32	i_mtime;	/* Modification time */
	u32	i_dtime;	/* Deletion Time */
	u16	i_gid;		/* Low 16 bits of Group Id */
	u16	i_links_count;	/* Links count */
	u32	i_blocks_lo;	/* Blocks count */
	u32	i_flags;	/* File flags */
	union {
		struct {
			u32  l_i_version;
		} linux1;
		struct {
			u32  h_i_translator;
		} hurd1;
		struct {
			u32  m_i_reserved1;
		} masix1;
	} osd1;				/* OS dependent 1 */
	u32	i_block[EXT4_N_BLOCKS];/* Pointers to blocks */
	u32	i_generation;	/* File version (for NFS) */
	u32	i_file_acl_lo;	/* File ACL */
	u32	i_size_high;
	u32	i_obso_faddr;	/* Obsoleted fragment address */
	union {
		struct {
			u16	l_i_blocks_high; /* were l_i_reserved1 */
			u16	l_i_file_acl_high;
			u16	l_i_uid_high;	/* these 2 fields */
			u16	l_i_gid_high;	/* were reserved2[0] */
			u16	l_i_checksum_lo;/* crc32c(uuid+inum+inode) LE */
			u16	l_i_reserved;
		} linux2;
		struct {
			u16	h_i_reserved1;	/* Obsoleted fragment number/size which are removed in ext4 */
			u16	h_i_mode_high;
			u16	h_i_uid_high;
			u16	h_i_gid_high;
			u32	h_i_author;
		} hurd2;
		struct {
			u16	h_i_reserved1;	/* Obsoleted fragment number/size which are removed in ext4 */
			u16	m_i_file_acl_high;
			u32	m_i_reserved2[2];
		} masix2;
	} osd2;				/* OS dependent 2 */
	u16	i_extra_isize;
	u16	i_checksum_hi;	/* crc32c(uuid+inum+inode) BE */
	u32  i_ctime_extra;  /* extra Change time      (nsec << 2 | epoch) */
	u32  i_mtime_extra;  /* extra Modification time(nsec << 2 | epoch) */
	u32  i_atime_extra;  /* extra Access time      (nsec << 2 | epoch) */
	u32  i_crtime;       /* File Creation time */
	u32  i_crtime_extra; /* extra FileCreationtime (nsec << 2 | epoch) */
	u32  i_version_hi;	/* high 32 bits for 64-bit version */
	u32	i_projid;	/* Project ID */
};

/*
 * Each block (leaves and indexes), even inode-stored has header.
 */
struct ext4_extent_header {
	u16	eh_magic;	/* probably will support different formats */
	u16	eh_entries;	/* number of valid entries */
	u16	eh_max;		/* capacity of store in entries */
	u16	eh_depth;	/* has tree real underlying blocks? */
	u32	eh_generation;	/* generation of the tree */
};

/*
 * This is the extent on-disk structure.
 * It's used at the bottom of the tree.
 */
struct ext4_extent {
	u32	ee_block;	/* first logical block extent covers */
	u16	ee_len;		/* number of blocks covered by extent */
	u16	ee_start_hi;	/* high 16 bits of physical block */
	u32	ee_start_lo;	/* low 32 bits of physical block */
};

/*
 * Structure of the super block
 */
struct ext4_super_block
{
	/*00*/ u32 s_inodes_count; /* Inodes count */
	u32	s_blocks_count_lo;	/* Blocks count */
	u32	s_r_blocks_count_lo;	/* Reserved blocks count */
	u32	s_free_blocks_count_lo;	/* Free blocks count */
/*10*/	u32	s_free_inodes_count;	/* Free inodes count */
	u32	s_first_data_block;	/* First Data Block */
	u32	s_log_block_size;	/* Block size */
	u32	s_log_cluster_size;	/* Allocation cluster size */
/*20*/	u32	s_blocks_per_group;	/* # Blocks per group */
	u32	s_clusters_per_group;	/* # Clusters per group */
	u32	s_inodes_per_group;	/* # Inodes per group */
	u32	s_mtime;		/* Mount time */
/*30*/	u32	s_wtime;		/* Write time */
	u16	s_mnt_count;		/* Mount count */
	u16	s_max_mnt_count;	/* Maximal mount count */
	u16	s_magic;		/* Magic signature */
	u16	s_state;		/* File system state */
	u16	s_errors;		/* Behaviour when detecting errors */
	u16	s_minor_rev_level;	/* minor revision level */
/*40*/	u32	s_lastcheck;		/* time of last check */
	u32	s_checkinterval;	/* max. time between checks */
	u32	s_creator_os;		/* OS */
	u32	s_rev_level;		/* Revision level */
/*50*/	u16	s_def_resuid;		/* Default uid for reserved blocks */
	u16	s_def_resgid;		/* Default gid for reserved blocks */
	/*
	 * These fields are for EXT4_DYNAMIC_REV superblocks only.
	 *
	 * Note: the difference between the compatible feature set and
	 * the incompatible feature set is that if there is a bit set
	 * in the incompatible feature set that the kernel doesn't
	 * know about, it should refuse to mount the filesystem.
	 *
	 * e2fsck's requirements are more strict; if it doesn't know
	 * about a feature in either the compatible or incompatible
	 * feature set, it must abort and not try to meddle with
	 * things it doesn't understand...
	 */
	u32	s_first_ino;		/* First non-reserved inode */
	u16  s_inode_size;		/* size of inode structure */
	u16	s_block_group_nr;	/* block group # of this superblock */
	u32	s_feature_compat;	/* compatible feature set */
/*60*/	u32	s_feature_incompat;	/* incompatible feature set */
	u32	s_feature_ro_compat;	/* readonly-compatible feature set */
/*68*/	u8	s_uuid[16];		/* 128-bit uuid for volume */
/*78*/	char	s_volume_name[16];	/* volume name */
/*88*/	char	s_last_mounted[64];	/* directory where last mounted */
/*C8*/	u32	s_algorithm_usage_bitmap; /* For compression */
	/*
	 * Performance hints.  Directory preallocation should only
	 * happen if the EXT4_FEATURE_COMPAT_DIR_PREALLOC flag is on.
	 */
	u8	s_prealloc_blocks;	/* Nr of blocks to try to preallocate*/
	u8	s_prealloc_dir_blocks;	/* Nr to preallocate for dirs */
	u16	s_reserved_gdt_blocks;	/* Per group desc for online growth */
	/*
	 * Journaling support valid if EXT4_FEATURE_COMPAT_HAS_JOURNAL set.
	 */
/*D0*/	u8	s_journal_uuid[16];	/* uuid of journal superblock */
/*E0*/	u32	s_journal_inum;		/* inode number of journal file */
	u32	s_journal_dev;		/* device number of journal file */
	u32	s_last_orphan;		/* start of list of inodes to delete */
	u32	s_hash_seed[4];		/* HTREE hash seed */
	u8	s_def_hash_version;	/* Default hash version to use */
	u8	s_jnl_backup_type;
	u16  s_desc_size;		/* size of group descriptor */
/*100*/	u32	s_default_mount_opts;
	u32	s_first_meta_bg;	/* First metablock block group */
	u32	s_mkfs_time;		/* When the filesystem was created */
	u32	s_jnl_blocks[17];	/* Backup of the journal inode */
	/* 64bit support valid if EXT4_FEATURE_COMPAT_64BIT */
/*150*/	u32	s_blocks_count_hi;	/* Blocks count */
	u32	s_r_blocks_count_hi;	/* Reserved blocks count */
	u32	s_free_blocks_count_hi;	/* Free blocks count */
	u16	s_min_extra_isize;	/* All inodes have at least # bytes */
	u16	s_want_extra_isize; 	/* New inodes should reserve # bytes */
	u32	s_flags;		/* Miscellaneous flags */
	u16  s_raid_stride;		/* RAID stride */
	u16  s_mmp_update_interval;  /* # seconds to wait in MMP checking */
	u64  s_mmp_block;            /* Block for multi-mount protection */
	u32  s_raid_stripe_width;    /* blocks on all data disks (N*stride)*/
	u8	s_log_groups_per_flex;  /* FLEX_BG group size */
	u8	s_checksum_type;	/* metadata checksum algorithm used */
	u8	s_encryption_level;	/* versioning level for encryption */
	u8	s_reserved_pad;		/* Padding to next 32bits */
	u64	s_kbytes_written;	/* nr of lifetime kilobytes written */
	u32	s_snapshot_inum;	/* Inode number of active snapshot */
	u32	s_snapshot_id;		/* sequential ID of active snapshot */
	u64	s_snapshot_r_blocks_count; /* reserved blocks for active
					      snapshot's future use */
	u32	s_snapshot_list;	/* inode number of the head of the
					   on-disk snapshot list */
//#define EXT4_S_ERR_START offsetof(struct ext4_super_block, s_error_count)
	u32	s_error_count;		/* number of fs errors */
	u32	s_first_error_time;	/* first time an error happened */
	u32	s_first_error_ino;	/* inode involved in first error */
	u64	s_first_error_block;	/* block involved of first error */
	u8	s_first_error_func[32];	/* function where the error happened */
	u32	s_first_error_line;	/* line number where error happened */
	u32	s_last_error_time;	/* most recent time of an error */
	u32	s_last_error_ino;	/* inode involved in last error */
	u32	s_last_error_line;	/* line number where error happened */
	u64	s_last_error_block;	/* block involved of last error */
	u8	s_last_error_func[32];	/* function where the error happened */
//#define EXT4_S_ERR_END offsetof(struct ext4_super_block, s_mount_opts)
	u8	s_mount_opts[64];
	u32	s_usr_quota_inum;	/* inode for tracking user quota */
	u32	s_grp_quota_inum;	/* inode for tracking group quota */
	u32	s_overhead_clusters;	/* overhead blocks/clusters in fs */
	u32	s_backup_bgs[2];	/* groups with sparse_super2 SBs */
	u8	s_encrypt_algos[4];	/* Encryption algorithms in use  */
	u8	s_encrypt_pw_salt[16];	/* Salt used for string2key algorithm */
	u32	s_lpf_ino;		/* Location of the lost+found inode */
	u32	s_prj_quota_inum;	/* inode for tracking project quota */
	u32	s_checksum_seed;	/* crc32c(uuid) if csum_seed set */
	u8	s_wtime_hi;
	u8	s_mtime_hi;
	u8	s_mkfs_time_hi;
	u8	s_lastcheck_hi;
	u8	s_first_error_time_hi;
	u8	s_last_error_time_hi;
	u8	s_pad[2];
	u16  s_encoding;		/* Filename charset encoding */
	u16  s_encoding_flags;	/* Filename charset encoding flags */
	u32	s_reserved[95];		/* Padding to the end of the block */
	u32	s_checksum;		/* crc32c(superblock) */
};

extern linux_virtaddr init_task;
extern unsigned long kaslr_offset;
extern linux_task_info init_task_info;
extern linux_virtaddr inode_in_qtd;
extern bool new_round;
extern bool wake_up_decoder;
extern struct ext4_super_block super_block;
extern int parse_inode_trigger;   //reset to 0 in usb_mscd.c csw phase

int read_virtaddr_data(linux_virtaddr virt_addr, int read_len, void *data);
int is_linux_user_addr(linux_virtaddr addr);
int is_linux_kernel_addr(linux_virtaddr addr);
// list
int get_list_head(linux_list_head list_head_addr, struct linux_list_head *list_head);
linux_virtaddr container_of(linux_virtaddr field_addr, ulong offset);
// list-end

//task
int get_linux_task_struct_info(linux_virtaddr task_struct, linux_task_info_ptr linux_task_info);
void print_task_info(linux_task_info_ptr linux_task_info);
int get_kaslr_offset(void);
int _get_init_task(void);
void get_init_task(void);
//int get_current_task(void);   /* Move to ransomtag_core.c */
linux_task_info traverse_task_list_by_inode(linux_task_info task_info, linux_virtaddr inode);
int find_cr3_in_task(linux_task_info task_info, u64 cr3);
linux_task_info traverse_task_list_by_cr3(linux_task_info task_info, u64 cr3);
linux_task_info find_cr3_in_task_list(u64 cr3);
// task-end

//files
int find_inode_in_task(linux_task_info task_info, linux_virtaddr inode);
linux_task_info find_inode_in_task_list(linux_virtaddr inode);
// files-end
u64 sg_page(linux_virtaddr page_link);
u64 get_urbvirt_from_qtdm(struct ehci_qtd_meta *qtdm);
u64 get_inode_from_qtdm(struct ehci_qtd_meta *qtdm);
//void analyse_qtd_linux(const struct mm_as *as, struct ehci_qtd_meta *qtdm);
int linux_ransomtag_storage_handle_sectors(struct storage_device *storage, struct storage_access *access, u8 *src, u8 *dst, int rw);
void linux_AddTag(struct cbw *cbw, unsigned int tag);
void linux_analyse_cbw(void *usb_msc_cbw);
/*************************  Linux End  ***************************************************************************************************/

/*************************  Windows Begin  ***************************************************************************************************/
#define STRUCT_KPCR_KPRCB_OFFSET                         0x20
#define STRUCT_KPRCB_CURRENT_THREAD_OFFSET               0x8
#define STRUCT_ETHREAD_PROCESS_OFFSET                    0x220
#define STRUCT_ETHREAD_IRPLIST_OFFSET                    0x670
#define STRUCT_EPROCESS_IMAGE_FILE_NAME_OFFSET           0x450
#define STRUCT_EPROCESS_DIRECTORY_TABLE_BASE_OFFSET      0x28
#define STRUCT_IRP_THREAD_LIST_ENTRY_OFFSET              0x20
#define STRUCT_IRP_TAIL_OFFSET                           0x78
#define STRUCT_IRP_STACK_COUNT_OFFSET                    0x42
#define STRUCT_IRP_CURRENT_LOCATION_OFFSET               0x43
#define IRP_TAIL_OVERLAY_OFFSET                          0x0
#define IRP_TAIL_OVERLAY_CURRENT_STACK_LOCATION_OFFSET   0x40
#define IRP_IO_STACK_LOCATION_OFFSET                     STRUCT_IRP_TAIL_OFFSET + IRP_TAIL_OVERLAY_OFFSET + IRP_TAIL_OVERLAY_CURRENT_STACK_LOCATION_OFFSET
#define IRP_TAIL_OVERLAY_ORIGINAL_FILE_OBJECT_OFFSET     0x48
#define IRP_ORIGINAL_FILE_OBJECT_OFFSET                  STRUCT_IRP_TAIL_OFFSET + IRP_TAIL_OVERLAY_OFFSET + IRP_TAIL_OVERLAY_ORIGINAL_FILE_OBJECT_OFFSET
#define STRUCT_IO_STACK_LOCATION_FILE_OBJECT_OFFSET      0x30
#define STRUCT_IO_STACK_LOCATION_LENGTH_OFFSET           0x8
#define STRUCT_IO_STACK_LOCATION_BYTEOFFSET_OFFSET       0x18
#define SIZEOF_STRUCT_IO_STACK_LOCATION                  0x48
#define STRUCT_FILE_OBJECT_FILENAME_OFFSET               0x58
#define STRUCT_UNICODE_STRING_BUFFER_OFFSET              0x8

#define WIN_KERNEL_ADDR_SPACE_START   0xFFFF000000000000
#define WIN_KERNEL_ADDR_SPACE_END     0xFFFFFFFFFFFFFFFF
#define IS_MFT            1
#define IS_BITMAP         2
#define CBW_DIRECTION_OUT    0
#define CBW_DIRECTION_IN     1

#define IRP_MJ_CREATE                   0x00
#define IRP_MJ_CLOSE                    0x02
#define IRP_MJ_READ                     0x03
#define IRP_MJ_WRITE                    0x04

#define WORD 8
#define SHIFT 3 
#define MASK 0x7 
#define NN 16000000
#define SectorToCluster(x) x/8
#define HIGH_ENTROPY     3
#define LOW_ENTROPY      2
#define INVALID_ENTROPY  1

//File activities
#define READFILE    1
#define WRITEFILE   2
#define DELETEFILE  3

//NTFS MetaFile
#define NTFS_BITMAP_START_LBA   2072
#define NTFS_BITMAP_END_LBA     2101
#define NTFS_MFT_START_LBA      319488
#define NTFS_MFT_END_LBA        320000

extern int SectorPageBitmap[1 + NN / WORD];

extern unsigned char bitmap[250000];
extern unsigned int bitmap_lba;
extern bool bitmap_inited;
extern bool bitmap_load_trigger;
extern bool bitmap_check_trigger;
extern bool bitmap_compare_trigger;
extern bool ParseMFT_trigger;
extern unsigned long long bitmap_offset;
extern unsigned int file_lba_start;
extern unsigned int file_lba_end;
extern long long entropy_length;


extern bool entropy_trigger;

typedef struct Win_UNICODE_STRING {
  unsigned short Length;
  Win_virtaddr  Buffer;
  unsigned char FileNameStr[512];
} Win_UNICODE_STRING;

struct win_list_entry {
    Win_virtaddr list_entry_field;
    Win_virtaddr Flink;
    Win_virtaddr Blink;
};

typedef struct win_file_object_info
{
    Win_virtaddr FileObject;
    Win_UNICODE_STRING FileName;
    unsigned long long ByteOffset;
    bool is_MFT;
    bool is_Bitmap;

}win_file_object_info;
typedef struct win_irp_info
{
    Win_virtaddr Irp;
    char StackCount;
    char CurrentLocation;
    Win_virtaddr CurrentStackLocation;
    Win_virtaddr TopStackLocation;
    unsigned char MajorFunction;
    win_file_object_info FileObjectInfo;
    unsigned long long ReadWriteLength;
    bool used;
} win_irp_info;
typedef struct win_process_info
{
    Win_virtaddr EProcess;
    Win_virtaddr EThread;
    struct win_list_entry IrpList;
    u64 DirectoryTableBase;
    u64 cr3;
    unsigned char ProcessName[15];
    struct win_irp_info IrpInfo[5];

} win_process_info;

typedef struct win_ransom_pattern
{
    unsigned char ProcessName[15];
    unsigned char ReadFileName[512];
    unsigned char WriteFileName[512];
    unsigned char DeleteFileName[512];
    bool used;
} win_ransom_pattern;
extern win_ransom_pattern patternlist[100];

//parse MFT
//#pragma pack(push,1)

typedef struct _FILE_RECORD_HEADER
{
	/*+0x00*/  u32 Type;            
	/*+0x04*/  u16 UsaOffset;        
	/*+0x06*/  u16 UsaCount;        
	/*+0x08*/  u64 Lsn;             
} FILE_RECORD_HEADER, *PFILE_RECORD_HEADER;


// 文件记录体  
typedef struct _FILE_RECORD {
	/*+0x00*/  FILE_RECORD_HEADER Ntfs;  
	/*+0x10*/  u16  SequenceNumber;   
	/*+0x12*/  u16  LinkCount;        
	/*+0x14*/  u16  AttributeOffset; 
	/*+0x16*/  u16  Flags;           
	/*+0x18*/  u32  BytesInUse;      
	/*+0x1C*/  u32  BytesAllocated;    
	/*+0x20*/  u64  BaseFileRecord;   
	/*+0x28*/  u16  NextAttributeNumber; 
	/*+0x2A*/  u16  Pading;           
	/*+0x2C*/  u32  MFTRecordNumber;  
	/*+0x30*/  u32  MFTUseFlags;     
}FILE_RECORD, *pFILE_RECORD;


// 属性类型定义  
typedef enum _ATTRIBUTE_TYPE
{
	AttributeStandardInformation = 0x10,
	AttributeAttributeList = 0x20,
	AttributeFileName = 0x30,
	AttributeObjectId = 0x40,
	AttributeSecurityDescriptor = 0x50,
	AttributeVolumeName = 0x60,
	AttributeVolumeInformation = 0x70,
	AttributeData = 0x80,
	AttributeIndexRoot = 0x90,
	AttributeIndexAllocation = 0xA0,
	AttributeBitmap = 0xB0,
	AttributeReparsePoint = 0xC0,
	AttributeEAInformation = 0xD0,
	AttributeEA = 0xE0,
	AttributePropertySet = 0xF0,
	AttributeLoggedUtilityStream = 0x100
} ATTRIBUTE_TYPE, *PATTRIBUTE_TYPE;



typedef struct
{
	/*+0x00*/  ATTRIBUTE_TYPE AttributeType;    
	/*+0x04*/  u16 RecordLength;             
	/**0x06*/  u16 unknow0;
	/*+0x08*/  unsigned char Nonresident;               
	/*+0x09*/  unsigned char NameLength;               

											  
										  
										  
	/*+0x0A*/  u16 NameOffset;          
												 
	/*+0x0C*/  u16 Flags;                    // ATTRIBUTE_xxx flags.  
	/*+0x0E*/  u16 AttributeNumber;          // The file-record-unique attribute instance number for this attribute.  
} ATTRIBUTE, *PATTRIBUTE;

// 属性头   
typedef struct _RESIDENT_ATTRIBUTE
{
	/*+0x00*/  ATTRIBUTE Attribute;  
	/*+0x10*/  u32 ValueLength;    
	/*+0x14*/  u16 ValueOffset;   
	/*+0x16*/  unsigned char Flags;           
	/*+0x17*/  unsigned char Padding0;        
} RESIDENT_ATTRIBUTE, *PRESIDENT_ATTRIBUTE;

// 文件属性ATTRIBUTE.AttributeType == 0x30  
typedef struct
{
	/*+0x00*/  u64 DirectoryFile : 48;    
	/*+0x06*/  u64 ReferenceNumber : 16;  
	/*+0x08*/  u64 CreationTime;        
	/*+0x10*/  u64 ChangeTime;                 
	/*+0x18*/  u64 LastWriteTime;       
	/*+0x20*/  u64 LastAccessTime;      
	/*+0x28*/  u64 AllocatedSize;       
	/*+0x30*/  u64 DataSize;            
	/*+0x38*/  u32 FileAttributes;      
	/*+0x3C*/  u32 AlignmentOrReserved; 
	/*+0x40*/  unsigned char NameLength;       

			   
	/*+0x41*/  unsigned char NameType;
	/*+0x42*/  unsigned short Name[1];         
} FILENAME_ATTRIBUTE, *PFILENAME_ATTRIBUTE;


//#pragma pack(pop)

#define MFT_FILE_SIZE (1024)

void win_ransomtag_init(void);
int is_win_kernel_addr(Win_virtaddr addr);
int win_read_virtaddr_data(Win_virtaddr virt_addr, int read_len, void *data);
Win_virtaddr win_container_of(Win_virtaddr list_entry_field, u64 offset);
Win_virtaddr win_get_KPCR(void);
Win_virtaddr win_get_KPRCB(void);
u64 win_get_cr3(void);
Win_virtaddr win_get_CurrentThread(void);
int win_get_CurrentProcessInfo(win_process_info *win_process_info);
void win_print_process_info(win_process_info *win_process_info);
int traverse_excluded_process_list(unsigned char *ProcessName);
int traverse_IrpList(win_process_info *CurrentProcessInfo, unsigned int lba);
void win_get_FileObjectInfo(win_file_object_info *win_file_object_info);
int win_is_MFT_or_Bitmap(unsigned char *FileNameStr, unsigned int lba);
void win_get_IrpInfo(win_irp_info *win_irp_info, unsigned int lba);
void win_print_irp_info(win_process_info *CurrentProcessInfo);
void analyse_qtd_windows(struct ehci_qtd_meta *qtdm);
int win_AddToSectorPageMap(u32 sector, u32 length);
void win_load_bitmap(void);
int win_AddToBitmap(unsigned char *buffer, unsigned long length);
int win_CheckBitmap(unsigned char *buffer, unsigned long length);
int win_SearchSectorMap(struct cbw *cbw);
int win_traverse_process_blacklist(unsigned char *ProcessName);
int win_add_to_process_blacklist(unsigned char *ProcessName);
void win_Handle_Recovery(struct cbw *cbw);
int ParseMFT(void *buf, unsigned int length);
int win_ransomtag_storage_handle_sectors(struct storage_device *storage, struct storage_access *access, u8 *src, u8 *dst, int rw);
void win_DetermineTag(struct cbw *cbw, win_process_info *CurrentProcessInfo);
void win_analyse(void *usb_msc_cbw);
/*************************  Windows End  ***************************************************************************************************/
#endifw
struct cbw {
	u32 dCBWSignature;
	u32 dCBWTag;
	u32 dCBWDataTransferLength;
	u8  bmCBWFlags;
	u8  bCBWLUN;			/* includes 4 reserved bits */
	u8  bCBWCBLength;		/* includes 3 reserved bits */
	u8 CBWCB[16];
};
typedef struct cbw_info
{
    struct cbw cbw;
    unsigned char direction;    // 0 is out, 1 is in
    u32 lba;
    u16 n_blocks;
} cbw_info;

void SetBitmap(int i);
void ClearBitmap(int i);
int TestBitmap(int i);
u32 reversebytes_uint32t(u32 value);
u64 reversebytes_uint64t(u64 value);
unsigned char *mem_find_str(unsigned char *full_data, int full_data_len, unsigned char *substr);


//qtd
int is_CBW(struct ehci_qtd_meta *qtdm, struct cbw_info *cbw_info);
void parse_CBW(struct cbw_info *cbw_info);
struct ehci_qtd *get_qtd_from_qtdm(struct ehci_qtd_meta *qtdm);
// qtd-end

//calculate entropy start
extern long long byte_count[256];
double __attribute__((target("sse"))) my_log(double a);
double __attribute__((target("sse"))) _entropy_calc(long long length);
int __attribute__((target("sse"))) entropy_calc(void);
int buffer_byte_count(u8 *data, long long  length);

// calculate entropy end
/*************************  Common End  ************************************************************************************************/




/*************************  Linux Begin  ***************************************************************************************************/
#define LINUX_ADDR_SIZE 8
#define LINUX_LISTHEAD_SIZE 16
#define SG_CHAIN 0x01UL
#define SG_END		0x02UL

#define INIT_TASK 0xffffffff82612780       //address in System.map
#define START_INIT_TASK 0xFFFFFFFF82600000    //address in System.map
#define CURRENT_TASK_GS_OFFSET 0x16BC0     //CURRENT_TASK offset in System.map

#define USER_ADDR_SPACE_START 0x0000000000000000
#define USER_ADDR_SPACE_END 0x00007FFFFFFFFFFF

#define KERNEL_ADDR_SPACE_START 0xFFFF800000000000
#define KERNEL_ADDR_SPACE_END 0xFFFFFFFFFFFFFFFF

//struct task_struct fields offset
#define TASK_STRUCT_COMM_OFFSET      0xA58     //task name
#define TASK_STRUCT_STACK_OFFSET     0x18      //task kernel stack
#define TASK_STRUCT_TASKS_OFFSET     0x7A0     //children tasks
#define TASK_STRUCT_PARENT_OFFSET    0x8B8     //parent
#define TASK_STRUCT_PID_OFFSET       0x8A0     //pid
#define TASK_STRUCT_FILES_OFFSET     0xAA0     //files
#define TASK_STRUCT_MM_OFFSET        0x7F0     //mm
#define TASK_STRUCT_ACTIVE_MM_OFFSET        0x7F8     //active_mm


//struct mm_struct fields offset
#define MM_STRUCT_PGD_OFFSET         0x50      //mm_struct->pgd

//struct files_struct fields offset
#define FILES_STRUCT_FDT_OFFSET      0x20      //files->fdt

//struct fdtable fields offset
#define FDTABLE_FD_OFFSET           0x8       //fdtable->fd
#define NR_OPEN_DEFAULT              64

//struct file fields offset
#define SIZEOF_STRUCT_FILE           256
#define STRUCT_FILE_FPATH_OFFSET     0x10      //file->f_path
#define STRUCT_FILE_FINODE_OFFSET    0x20      //file->f_inode
#define STRUCT_FILE_FMAPPING_OFFSET  0xF0      //file->f_mapping

struct scatterlist {
	linux_virtaddr	page_link;
	unsigned int	offset;
	unsigned int	length;
	linux_virtaddr	dma_address;
};

struct linux_list_head {
    linux_virtaddr list_head_field;
    linux_virtaddr next;
    linux_virtaddr prev;
};

typedef struct linux_task_info
{
    linux_virtaddr task_struct;
    linux_virtaddr stack;
    linux_virtaddr files;
    linux_virtaddr fdtable;
    struct linux_list_head list_head_tasks;
    u32 pid;
    linux_virtaddr parent_task_struct;
    u32 parent_pid;
    linux_virtaddr mm;
    linux_virtaddr mm_pgd;
    u64 mm_pgd_phys;
    linux_virtaddr active_mm;
    linux_virtaddr active_mm_pgd;
    u64 active_mm_pgd_phys;
    char task_name[32];

} linux_task_info, *linux_task_info_ptr;

extern linux_virtaddr init_task;
extern unsigned long kaslr_offset;
extern linux_task_info init_task_info;
extern linux_virtaddr inode_in_qtd;
extern bool new_round;
extern bool wake_up_decoder;

int read_virtaddr_data(linux_virtaddr virt_addr, int read_len, void *data);
int is_linux_user_addr(linux_virtaddr addr);
int is_linux_kernel_addr(linux_virtaddr addr);
// list
int get_list_head(linux_list_head list_head_addr, struct linux_list_head *list_head);
linux_virtaddr container_of(linux_virtaddr field_addr, ulong offset);
// list-end

//task
int get_linux_task_struct_info(linux_virtaddr task_struct, linux_task_info_ptr linux_task_info);
void print_task_info(linux_task_info_ptr linux_task_info);
int get_kaslr_offset(void);
int _get_init_task(void);
void get_init_task(void);
int get_current_task(void);
linux_task_info traverse_task_list_by_inode(linux_task_info task_info, linux_virtaddr inode);
int find_cr3_in_task(linux_task_info task_info, u64 cr3);
linux_task_info traverse_task_list_by_cr3(linux_task_info task_info, u64 cr3);
linux_task_info find_cr3_in_task_list(u64 cr3);
// task-end

//files
int find_inode_in_task(linux_task_info task_info, linux_virtaddr inode);
linux_task_info find_inode_in_task_list(linux_virtaddr inode);
// files-end
u64 sg_page(linux_virtaddr page_link);
u64 get_urbvirt_from_qtdm(struct ehci_qtd_meta *qtdm);
u64 get_inode_from_qtdm(struct ehci_qtd_meta *qtdm);
void analyse_qtd_linux(struct ehci_qtd_meta *qtdm);
/*************************  Linux End  ***************************************************************************************************/





/*************************  Windows Begin  ***************************************************************************************************/
#define STRUCT_KPCR_KPRCB_OFFSET                         0x20
#define STRUCT_KPRCB_CURRENT_THREAD_OFFSET               0x8
#define STRUCT_ETHREAD_PROCESS_OFFSET                    0x220
#define STRUCT_ETHREAD_IRPLIST_OFFSET                    0x670
#define STRUCT_EPROCESS_IMAGE_FILE_NAME_OFFSET           0x450
#define STRUCT_EPROCESS_DIRECTORY_TABLE_BASE_OFFSET      0x28
#define STRUCT_IRP_THREAD_LIST_ENTRY_OFFSET              0x20
#define STRUCT_IRP_TAIL_OFFSET                           0x78
#define STRUCT_IRP_STACK_COUNT_OFFSET                    0x42
#define STRUCT_IRP_CURRENT_LOCATION_OFFSET               0x43
#define IRP_TAIL_OVERLAY_OFFSET                          0x0
#define IRP_TAIL_OVERLAY_CURRENT_STACK_LOCATION_OFFSET   0x40
#define IRP_IO_STACK_LOCATION_OFFSET                     STRUCT_IRP_TAIL_OFFSET + IRP_TAIL_OVERLAY_OFFSET + IRP_TAIL_OVERLAY_CURRENT_STACK_LOCATION_OFFSET
#define IRP_TAIL_OVERLAY_ORIGINAL_FILE_OBJECT_OFFSET     0x48
#define IRP_ORIGINAL_FILE_OBJECT_OFFSET                  STRUCT_IRP_TAIL_OFFSET + IRP_TAIL_OVERLAY_OFFSET + IRP_TAIL_OVERLAY_ORIGINAL_FILE_OBJECT_OFFSET
#define STRUCT_IO_STACK_LOCATION_FILE_OBJECT_OFFSET      0x30
#define STRUCT_IO_STACK_LOCATION_LENGTH_OFFSET           0x8
#define STRUCT_IO_STACK_LOCATION_BYTEOFFSET_OFFSET       0x18
#define SIZEOF_STRUCT_IO_STACK_LOCATION                  0x48
#define STRUCT_FILE_OBJECT_FILENAME_OFFSET               0x58
#define STRUCT_UNICODE_STRING_BUFFER_OFFSET              0x8

#define WIN_KERNEL_ADDR_SPACE_START   0xFFFF000000000000
#define WIN_KERNEL_ADDR_SPACE_END     0xFFFFFFFFFFFFFFFF
#define IS_MFT            1
#define IS_BITMAP         2
#define CBW_DIRECTION_OUT    0
#define CBW_DIRECTION_IN     1

#define IRP_MJ_CREATE                   0x00
#define IRP_MJ_CLOSE                    0x02
#define IRP_MJ_READ                     0x03
#define IRP_MJ_WRITE                    0x04

#define WORD 8
#define SHIFT 3 
#define MASK 0x7 
#define NN 16000000
#define SectorToCluster(x) x/8
#define HIGH_ENTROPY     3
#define LOW_ENTROPY      2
#define INVALID_ENTROPY  1

//File activities
#define READFILE    1
#define WRITEFILE   2
#define DELETEFILE  3

//NTFS MetaFile
#define NTFS_BITMAP_START_LBA   2072
#define NTFS_BITMAP_END_LBA     2101
#define NTFS_MFT_START_LBA      319488
#define NTFS_MFT_END_LBA        320000

extern int SectorPageBitmap[1 + NN / WORD];

extern unsigned char bitmap[250000];
extern unsigned int bitmap_lba;
extern bool bitmap_inited;
extern bool bitmap_load_trigger;
extern bool bitmap_check_trigger;
extern bool bitmap_compare_trigger;
extern bool ParseMFT_trigger;
extern unsigned long long bitmap_offset;
extern unsigned int file_lba_start;
extern unsigned int file_lba_end;
extern long long entropy_length;


extern bool entropy_trigger;

typedef struct Win_UNICODE_STRING {
  unsigned short Length;
  Win_virtaddr  Buffer;
  unsigned char FileNameStr[512];
} Win_UNICODE_STRING;

struct win_list_entry {
    Win_virtaddr list_entry_field;
    Win_virtaddr Flink;
    Win_virtaddr Blink;
};

typedef struct win_file_object_info
{
    Win_virtaddr FileObject;
    Win_UNICODE_STRING FileName;
    unsigned long long ByteOffset;
    bool is_MFT;
    bool is_Bitmap;

}win_file_object_info;
typedef struct win_irp_info
{
    Win_virtaddr Irp;
    char StackCount;
    char CurrentLocation;
    Win_virtaddr CurrentStackLocation;
    Win_virtaddr TopStackLocation;
    unsigned char MajorFunction;
    win_file_object_info FileObjectInfo;
    unsigned long long ReadWriteLength;
    bool used;
} win_irp_info;
typedef struct win_process_info
{
    Win_virtaddr EProcess;
    Win_virtaddr EThread;
    struct win_list_entry IrpList;
    u64 DirectoryTableBase;
    u64 cr3;
    unsigned char ProcessName[15];
    struct win_irp_info IrpInfo[5];

} win_process_info;

typedef struct win_ransom_pattern
{
    unsigned char ProcessName[15];
    unsigned char ReadFileName[512];
    unsigned char WriteFileName[512];
    unsigned char DeleteFileName[512];
    bool used;
} win_ransom_pattern;
extern win_ransom_pattern patternlist[100];

//parse MFT
//#pragma pack(push,1)
// 文件记录头  
typedef struct _FILE_RECORD_HEADER
{
	/*+0x00*/  u32 Type;            
	/*+0x04*/  u16 UsaOffset;     
	/*+0x06*/  u16 UsaCount;        
	/*+0x08*/  u64 Lsn;             
} FILE_RECORD_HEADER, *PFILE_RECORD_HEADER;


// 文件记录体  
typedef struct _FILE_RECORD {
	/*+0x00*/  FILE_RECORD_HEADER Ntfs; 
	/*+0x10*/  u16  SequenceNumber;   
	/*+0x12*/  u16  LinkCount;        
	/*+0x14*/  u16  AttributeOffset; 
	/*+0x16*/  u16  Flags;            
	/*+0x18*/  u32  BytesInUse;      
	/*+0x1C*/  u32  BytesAllocated;  
	/*+0x20*/  u64  BaseFileRecord; 
	/*+0x28*/  u16  NextAttributeNumber; 
	/*+0x2A*/  u16  Pading;           
	/*+0x2C*/  u32  MFTRecordNumber; 
	/*+0x30*/  u32  MFTUseFlags;     
}FILE_RECORD, *pFILE_RECORD;


// 属性类型定义  
typedef enum _ATTRIBUTE_TYPE
{
	AttributeStandardInformation = 0x10,
	AttributeAttributeList = 0x20,
	AttributeFileName = 0x30,
	AttributeObjectId = 0x40,
	AttributeSecurityDescriptor = 0x50,
	AttributeVolumeName = 0x60,
	AttributeVolumeInformation = 0x70,
	AttributeData = 0x80,
	AttributeIndexRoot = 0x90,
	AttributeIndexAllocation = 0xA0,
	AttributeBitmap = 0xB0,
	AttributeReparsePoint = 0xC0,
	AttributeEAInformation = 0xD0,
	AttributeEA = 0xE0,
	AttributePropertySet = 0xF0,
	AttributeLoggedUtilityStream = 0x100
} ATTRIBUTE_TYPE, *PATTRIBUTE_TYPE;


// 属性头  
typedef struct
{
	/*+0x00*/  ATTRIBUTE_TYPE AttributeType;   
	/*+0x04*/  u16 RecordLength;           
	/**0x06*/  u16 unknow0;
	/*+0x08*/  unsigned char Nonresident;             
	/*+0x09*/  unsigned char NameLength;                

											 
										 
										 
	/*+0x0A*/  u16 NameOffset;          
										  
	/*+0x0C*/  u16 Flags;                   
	/*+0x0E*/  u16 AttributeNumber;         
} ATTRIBUTE, *PATTRIBUTE;

// 属性头   
typedef struct _RESIDENT_ATTRIBUTE
{
	/*+0x00*/  ATTRIBUTE Attribute;   
	/*+0x10*/  u32 ValueLength;     
	/*+0x14*/  u16 ValueOffset;    
	/*+0x16*/  unsigned char Flags;        
	/*+0x17*/  unsigned char Padding0;        
} RESIDENT_ATTRIBUTE, *PRESIDENT_ATTRIBUTE;


typedef struct
{
	/*+0x00*/  u64 DirectoryFile : 48;    
	/*+0x06*/  u64 ReferenceNumber : 16;  
	/*+0x08*/  u64 CreationTime;      
	/*+0x10*/  u64 ChangeTime;                  
	/*+0x18*/  u64 LastWriteTime;      
	/*+0x20*/  u64 LastAccessTime;      
	/*+0x28*/  u64 AllocatedSize;      
	/*+0x30*/  u64 DataSize;            
	/*+0x38*/  u32 FileAttributes;      
	/*+0x3C*/  u32 AlignmentOrReserved; 
	/*+0x40*/  unsigned char NameLength;     

			   
	/*+0x41*/  unsigned char NameType;
	/*+0x42*/  unsigned short Name[1];        
} FILENAME_ATTRIBUTE, *PFILENAME_ATTRIBUTE;


//#pragma pack(pop)

#define MFT_FILE_SIZE (1024)

void win_ransomtag_init(void);
int is_win_kernel_addr(Win_virtaddr addr);
int win_read_virtaddr_data(Win_virtaddr virt_addr, int read_len, void *data);
Win_virtaddr win_container_of(Win_virtaddr list_entry_field, u64 offset);
Win_virtaddr win_get_KPCR(void);
Win_virtaddr win_get_KPRCB(void);
u64 win_get_cr3(void);
Win_virtaddr win_get_CurrentThread(void);
int win_get_CurrentProcessInfo(win_process_info *win_process_info);
void win_print_process_info(win_process_info *win_process_info);
int traverse_excluded_process_list(unsigned char *ProcessName);
int traverse_IrpList(win_process_info *CurrentProcessInfo, unsigned int lba);
void win_get_FileObjectInfo(win_file_object_info *win_file_object_info);
int win_is_MFT_or_Bitmap(unsigned char *FileNameStr, unsigned int lba);
void win_get_IrpInfo(win_irp_info *win_irp_info, unsigned int lba);
void win_print_irp_info(win_process_info *CurrentProcessInfo);
void analyse_qtd_windows(struct ehci_qtd_meta *qtdm);
int win_AddToSectorPageMap(u32 sector, u32 length);
void win_load_bitmap(void);
int win_AddToBitmap(unsigned char *buffer, unsigned long length);
int win_CheckBitmap(unsigned char *buffer, unsigned long length);
int win_SearchSectorMap(struct cbw *cbw);
int win_traverse_process_blacklist(unsigned char *ProcessName);
int win_add_to_process_blacklist(unsigned char *ProcessName);
void win_Handle_Recovery(struct cbw *cbw);
int ParseMFT(void *buf, unsigned int length);
int win_ransomtag_storage_handle_sectors(struct storage_device *storage, struct storage_access *access, u8 *src, u8 *dst, int rw);
void win_DetermineTag(struct cbw *cbw, win_process_info *CurrentProcessInfo);
void win_analyse(void *usb_msc_cbw);
/*************************  Windows End  ***************************************************************************************************/
#endif