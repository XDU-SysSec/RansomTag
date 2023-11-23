// #include <core.h>
#include <usb.h>
#include <usb_device.h>
#include <usb_hook.h>
#include "ehci.h"
#include "ehci_debug.h"
#include "ransomtag.h"
#include "../core/cpu_mmu.h"
#include <storage.h>
#include "../core/ransomtag_core.h"
#include "../core/thread.h"
#include <core/printf.h>
#include <core/mm.h>

#define L2B(buf,offset) buf[offset] + (buf[offset + 1] << 8) \
			+ (buf[offset + 2] << 16) + (buf[offset + 3] << 24)

unsigned int linux_need_tag = 0;
linux_virtaddr init_task = 0;
linux_task_info init_task_info;
long long byte_count[256] = {0};
linux_virtaddr inode_in_qtd = 0;
bool new_round = false;
bool wake_up_decoder = false;
bool test_start = true;
unsigned int file_lba_start = 0; // using for entropy trigger
unsigned int file_lba_end = 0;   // using for entropy trigger
unsigned char ProcessForEntropyCalc[15] = {0};
unsigned char ProcessForPattern[15] = {0};
long long entropy_length = 0;
unsigned long kaslr_offset = 0; // Kernel address space layout randomization


//Linux
bool read_ext4_metadata = false;
bool super_block_loaded = false;
bool read_group_desc_metadata = false;
bool group_desc_loaded = false;
u64 sector_index = 0;

unsigned char win_excluded_processes[][15] = {
    "Idle",
    "7zG",
    "WinRAR",
    //"System",
    //"explorer",
    // "svchost.exe",
    // "SearchIndexer",
    "DbgView.exe"};

unsigned char win_entropy_excluded_processes[][15] = {
    "Idle",
    "System",
    "explorer",
    // "svchost.exe",
    // "SearchIndexer",
    "DbgView.exe"};
unsigned char win_processe_blacklist[100][15] = {0};
unsigned char win_high_entropylist[100][15] = {0};
int blacklist_count = 0;

unsigned char NTFS_MetaFile_MFT[10] = {0x5C, 0x0, 0x24, 0x0, 0x4D, 0x0, 0x66, 0x0, 0x74};
unsigned char NTFS_MetaFile_Bitmap[16] = {0x5C, 0x0, 0x24, 0x0, 0x42, 0x0, 0x69, 0x0, 0x74, 0x0, 0x4D, 0x0, 0x61, 0x0, 0x70};
unsigned char NTFS_MetaFile_ConvertToNonresident[43] =
    {0x24, 0x0, 0x43, 0x0, 0x6F, 0x0, 0x6E, 0x0, 0x76, 0x0, 0x65, 0x0, 0x72, 0x0, 0x74, 0x0,
     0x54, 0x0, 0x6F, 0x0, 0x4E, 0x0, 0x6F, 0x0, 0x6E, 0x0, 0x72, 0x0, 0x65, 0x0, 0x73, 0x0,
     0x69, 0x0, 0x64, 0x0, 0x65, 0x0, 0x6E, 0x0, 0x74, 0x0};
unsigned char bitmap[250000];
unsigned int bitmap_lba = 0;
unsigned long long bitmap_offset;
bool bitmap_inited = false;
bool bitmap_load_trigger = false;
bool bitmap_check_trigger = false;
bool bitmap_compare_trigger = false;
bool byte_count_trigger = false;
bool ParseMFT_trigger = false;
int SectorPageBitmap[1 + NN / WORD] = {0};
int sector2048count = 0;

bool entropy_trigger = false;
win_ransom_pattern patternlist[100];

/*************************  Common Start  ************************************************************************************************/
/*
 * 
 */
void SetBitmap(int i)
{
    SectorPageBitmap[i >> SHIFT] |= (1 << (i & MASK));
}

void ClearBitmap(int i)
{
    SectorPageBitmap[i >> SHIFT] &= ~(1 << (i & MASK));
}

int TestBitmap(int i)
{
    return SectorPageBitmap[i >> SHIFT] & (1 << (i & MASK));
} /************SectorPageBitMap*****************/

u32 reversebytes_uint32t(u32 value)
{
    return (value & 0x000000FFU) << 24 | (value & 0x0000FF00U) << 8 |
           (value & 0x00FF0000U) >> 8 | (value & 0xFF000000U) >> 24;
}


u64 reversebytes_uint64t(u64 value)
{
    u32 high_uint64 = (u64)(reversebytes_uint32t((u32)(value)));    // 低32位转成小端
    u64 low_uint64 = (u64)reversebytes_uint32t((u32)(value >> 32)); // 高32位转成小端
    return (high_uint64 << 32) + low_uint64;
}

static inline u16 bswap16(u16 x)
{
    return (x << 8) | (x >> 8);
}

unsigned char *mem_find_str(unsigned char *full_data, int full_data_len, unsigned char *substr)
{
    if (full_data == NULL || full_data_len <= 0 || substr == NULL)
    {
        return NULL;
    }

    if (*substr == '\0')
    {
        return NULL;
    }

    int sublen = strlen(substr);
    int i = 0;
    char *cur = full_data;
    int last_possible = full_data_len - sublen + 1;
    for (i = 0; i < last_possible; i++)
    {
        if (*cur == *substr)
        {
            if (memcmp(cur, substr, sublen) == 0)
                return cur;
        }
        cur++;
    }

    return NULL;
}

// address-end

// qtd
/*
int is_CBW(struct ehci_qtd_meta *qtdm, struct cbw_info *cbw_info)
{
    int len = (qtdm->qtd->token & 0x7fff0000U) >> 16;
    if (len == 31)
    {
        virt_t vadr;

        // map a guest buffer into the vm area
        vadr = (virt_t)mapmem_as(as, qtdm->qtd->buffer[0], 4, 0);

        ASSERT(vadr);

        // double check
        if (memcmp((char *)vadr, "USBC", 4) == 0)
        {
            printf("%s: is a CBW  qtd len:%d\n", __func__, len);
            if(cbw_info)
            {
                memcpy((void *)&cbw_info->cbw, (void *)vadr, sizeof(cbw_info->cbw));
            }
            unmapmem((void *)vadr, 4);
            return 1;
        }
        unmapmem((void *)vadr, 4);
    }

    return 0;
}
*/


void parse_CBW(struct cbw_info *cbw_info)
{
    if (!cbw_info)
        return;

    struct cbw *cbw;
    cbw = &cbw_info->cbw;
    cbw_info->direction = cbw->bmCBWFlags >> 7;
    cbw_info->lba = (cbw->CBWCB[2] << 24) | (cbw->CBWCB[3] << 16) | (cbw->CBWCB[4] << 8) | (cbw->CBWCB[5] << 0); /* big endian */
    cbw_info->n_blocks = bswap16(*(u16 *)&cbw->CBWCB[7]);                                                        /* big endian */
    printf("%s: Direction: %d, LBA=%08X, NBLK=%04X, DataLenth: %d\n", __func__, cbw_info->direction, cbw_info->lba, cbw_info->n_blocks, cbw_info->cbw.dCBWDataTransferLength);
}

struct ehci_qtd *get_qtd_from_qtdm(struct ehci_qtd_meta *qtdm)
{
    if (qtdm->ovlay)
        return qtdm->ovlay;
    else
        return qtdm->qtd;
}
// qtd-end

// calculate entropy start
double __attribute__((target("sse"))) my_log(double a)
{
    int N = 40;
    int k, nk;
    double x, xx, y;
    x = (a - 1) / (a + 1);
    xx = x * x;
    nk = 2 * N + 1;
    y = 1.0 / nk;
    for (k = N; k > 0; k--)
    {
        nk = nk - 2;
        y = 1.0 / nk + xx * y;
    }
    return 2.0 * x * y;
}

double __attribute__((target("sse"))) _entropy_calc(long long length)
{
    double entropy = 0;
    double count = 0;
    int i;

    length = length - byte_count[0];

    for (i = 1; i < 256; i++)
    {
        if (byte_count[i] != 0)
        {
            count = (double)byte_count[i] / (double)length;
            entropy += (-(count * (my_log(count) / my_log(2))));
            // printf("%s: byte_count[%d]: %ld, entropy:%d\n", __func__, i, byte_count[i], (int)entropy);
        }
    }

    return entropy;
}

int __attribute__((target("sse"))) entropy_calc(void)
{
    double entropy = 0;

    if (entropy_length == 0)
    {
        printf("%s: entropy ERROR! entropy_length==0\n", __func__);
        return 0;
    }

    entropy = _entropy_calc(entropy_length);

    if (entropy >= 4)
    {
        printf("%s: Process [%s] entropy is HIGH, length=%lld\n", __func__, ProcessForEntropyCalc, entropy_length);
        return HIGH_ENTROPY;
    }
    else if (entropy >= 0 && entropy < 4)
    {
        printf("%s: Process [%s] entropy is LOW, length=%lld\n", __func__, ProcessForEntropyCalc, entropy_length);
        return LOW_ENTROPY;
    }
    else if (entropy < 0)
    {
        printf("%s: Process [%s] entropy is INVALID, length=%lld\n", __func__, ProcessForEntropyCalc, entropy_length);
        return INVALID_ENTROPY;
    }

    return 0;
}

int buffer_byte_count(u8 *data, long long length)
{
    unsigned long i;
    for (i = 0; i < length; i++)
    {
        if ((int)data[i] != 0

        )
            byte_count[(int)data[i]]++;
    }

    return 1;
}
// calculate entropy end
/*************************  Common End  ************************************************************************************************/

/*************************  Linux Begin  ***************************************************************************************************/
#define SECTOR_TO_EXT4_BLOCK(Sector) (Sector * 512 / 4096)
#define EXT4_BLOCK_TO_SECTOR_START(Block)  (Block * 8)
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))

struct ext4_super_block super_block = {0};
u32 group_total = 0;
struct ext4_group_desc group_desc[4] = {0};
int loaded_group_desc_count = 0;
int inode_table_start_sector[4] = {0};
int trigger_mfttag_start = 0;
int trigger_mfttag_end = 0;
int trigger_recovertag_start = 0;
int trigger_recovertag_end = 0;
int parse_inode_trigger = 0;
int itable_transfer_length = 0;
unsigned int dir_inode_extent_block[500] = {0};  //for recording dir_inodes which need to be tagged with MFT tag
u32 inode_bitmap_blocks[4] = {0};

int read_virtaddr_data(linux_virtaddr virt_addr, int read_len, void *data)
{
    
    if(!is_linux_kernel_addr(virt_addr) || !data)
    {
        return 0;
    }
    

    //unsigned char *data_array = (unsigned char *)data;
	for (int i = 0; i < read_len; i++)
	{
		read_linearaddr_b(virt_addr, (unsigned char *)data);
		virt_addr++;
        (unsigned char *)data++;
    }
    return 1;
}

int is_linux_user_addr(linux_virtaddr addr)
{
    int ret = 0;
    if(addr >= USER_ADDR_SPACE_START && addr <  USER_ADDR_SPACE_END)
    {
        ret = 1;
    }
    else
    {
        //printf("%s virt_addr[0x%llX] is not a user addr!!!\n", __func__, addr);
        ret = 0;
    }
    return ret;
}

int is_linux_kernel_addr(linux_virtaddr addr)
{
    int ret = 0;
    if(addr >= KERNEL_ADDR_SPACE_START && addr <= KERNEL_ADDR_SPACE_END)
    {
        ret = 1;
    }
    else
    {
        //printf("%s virt_addr[0x%llX] is not a kernel addr!!!\n", __func__, addr);
        ret = 0;
    }
    return ret;
}

//list
int get_list_head(linux_list_head list_head_addr, struct linux_list_head *list_head)
{
    if(!is_linux_kernel_addr(list_head_addr))
    {
        return 0;
    }

    linux_virtaddr next;
    linux_virtaddr prev;
    read_virtaddr_data(list_head_addr, LINUX_ADDR_SIZE, &next);
    read_virtaddr_data(list_head_addr + LINUX_ADDR_SIZE, LINUX_ADDR_SIZE, &prev);
    list_head->next = next;
    list_head->prev = prev;

    return 1;
}

linux_virtaddr container_of(linux_virtaddr field_addr,ulong offset)
{
    return field_addr - offset;
}
//list-end


//task
int get_linux_task_struct_info(linux_virtaddr task_struct,linux_task_info_ptr linux_task_info)  
{
    int ret = 0;
    linux_task_info->task_struct = task_struct;
    
    read_virtaddr_data(task_struct + TASK_STRUCT_STACK_OFFSET, LINUX_ADDR_SIZE, &linux_task_info->stack);
    read_virtaddr_data(task_struct + TASK_STRUCT_FILES_OFFSET, LINUX_ADDR_SIZE, &linux_task_info->files);
    read_virtaddr_data(linux_task_info->files + FILES_STRUCT_FDT_OFFSET, LINUX_ADDR_SIZE, &linux_task_info->fdtable);
    read_virtaddr_data(task_struct + TASK_STRUCT_TASKS_OFFSET, LINUX_ADDR_SIZE, &linux_task_info->list_head_tasks.next);
    read_virtaddr_data(task_struct + TASK_STRUCT_TASKS_OFFSET + LINUX_ADDR_SIZE, LINUX_ADDR_SIZE, &linux_task_info->list_head_tasks.prev);
    read_virtaddr_data(task_struct + TASK_STRUCT_PID_OFFSET, LINUX_ADDR_SIZE, &linux_task_info->pid);
    read_virtaddr_data(task_struct + TASK_STRUCT_PARENT_OFFSET, LINUX_ADDR_SIZE, &linux_task_info->parent_task_struct);
    read_virtaddr_data(linux_task_info->parent_task_struct + TASK_STRUCT_PID_OFFSET, LINUX_ADDR_SIZE, &linux_task_info->parent_pid);
    read_virtaddr_data(task_struct + TASK_STRUCT_COMM_OFFSET,16,linux_task_info->task_name);
    read_virtaddr_data(task_struct + TASK_STRUCT_MM_OFFSET, LINUX_ADDR_SIZE, &linux_task_info->mm);
    read_virtaddr_data(linux_task_info->mm + MM_STRUCT_PGD_OFFSET, LINUX_ADDR_SIZE, &linux_task_info->mm_pgd);
    read_virtaddr_data(task_struct + TASK_STRUCT_ACTIVE_MM_OFFSET, LINUX_ADDR_SIZE, &linux_task_info->active_mm);
    read_virtaddr_data(linux_task_info->active_mm + MM_STRUCT_PGD_OFFSET, LINUX_ADDR_SIZE, &linux_task_info->active_mm_pgd);

    linux_task_info->mm_pgd_phys = guestvirt_to_guestphys(linux_task_info->mm_pgd);
    linux_task_info->active_mm_pgd_phys = guestvirt_to_guestphys(linux_task_info->active_mm_pgd);

    linux_task_info->list_head_tasks.list_head_field = task_struct + TASK_STRUCT_TASKS_OFFSET;

    return ret;
}


void print_task_info(linux_task_info_ptr linux_task_info)
{
    if (linux_task_info)
    {

        printf("<<<<<<<<<<<<<<<<<<<task_info>>>>>>>>>>>>>>>>>>>\n");
        printf("             task_name:  %s\n", linux_task_info->task_name);
        printf("                   pid:  %d\n", linux_task_info->pid);
        printf("            parent_pid:  %d\n", linux_task_info->parent_pid);
        printf("           task_struct:  0x%llX\n", linux_task_info->task_struct);
        printf("    parent_task_struct:  0x%llX\n", linux_task_info->parent_task_struct);
        printf("                 stack:  0x%llX\n", linux_task_info->stack);
        printf(" list_head_tasks.field:  0x%llX\n", linux_task_info->list_head_tasks.list_head_field);
        printf("  list_head_tasks.next:  0x%llX\n", linux_task_info->list_head_tasks.next);
        printf("  list_head_tasks.prev:  0x%llX\n", linux_task_info->list_head_tasks.prev);
        printf("<<<<<<<<<<<<<<<<<<<<<<end>>>>>>>>>>>>>>>>>>>>>>\n");
    }
}

int get_kaslr_offset(void)
{
	unsigned long init_task_stack = 0;
	int ret = get_init_task_stack(&init_task_stack);
	if (ret < 0)
	{
		return -1;
	}
	else
	{
		kaslr_offset = init_task_stack - START_INIT_TASK;
        printf("kaslr_offset= 0x%llX\n", kaslr_offset);
        return 1;
    }
}

int _get_init_task(void)
{
	
	int ret = get_kaslr_offset();
	if(ret <0)
	{
		return 0;
	}
	else
	{
		init_task = INIT_TASK + kaslr_offset;
        printf("Find init_task!!\n");
        return 1;
    }
}

void get_init_task(void)
{
    if (init_task == 0)
    {
        int ret = _get_init_task();
        if (ret)
        {
            printf("init_task=0x%llX\n", init_task);
            get_linux_task_struct_info(init_task, &init_task_info);
            print_task_info(&init_task_info);
        }
    }
}

/* Move to ransomtag_core.c */
/*
int get_current_task(void)
{
	// get stack from rbp reg 
	ulong rsp;
	ulong rbp;
	get_current_rsp_reg(&rsp, &rbp); 
	if (rsp > 0xFFFF000000000000)
	{
		printf("%s: rsp:[0x%llX], rbp:[0x%llX]\n", __func__, rsp, rbp);
		printf("%s: rsp->stack:[0x%llX], rbp->stack:[0x%llX]\n", __func__, rsp & ~(0x4000 - 1), rbp & ~(0x4000 - 1));
	}
	
    
	ulong gs;
	get_current_gs_reg(&gs);
	ulong current_task = gs + CURRENT_TASK_GS_OFFSET;
	linux_virtaddr task_struct_addr;
    int ret = 0;

    ret = read_virtaddr_data(current_task, LINUX_ADDR_SIZE, &task_struct_addr);
    printf("%s: gs_base:0x%llX, current_task: 0x%llX\n", __func__, gs, current_task);
    if(ret && is_linux_kernel_addr(task_struct_addr))
	{
        linux_task_info task_info;
        get_linux_task_struct_info(task_struct_addr, &task_info);
        if (!linux_traverse_excluded_process_list(task_info.task_name))
        {
            print_task_info(&task_info);
        }
        
    }
    
    return 0;
}
*/

linux_task_info traverse_task_list_by_inode(linux_task_info task_info, linux_virtaddr inode)
{

    linux_task_info pos;
    linux_virtaddr task_struct;
    ulong count = 0;
    int ret = 0;

    for (pos = task_info; pos.list_head_tasks.next != task_info.list_head_tasks.list_head_field;)
    {
        count++;
        if(pos.parent_pid!=2)
        {
            ret = find_inode_in_task(pos, inode);
            if(ret==1)
            {
                printf("%s: FIND!!! task_name=%s\n", __func__, pos.task_name);

                print_task_info(&task_info);

                return pos;
            }
        }
        task_struct = container_of(pos.list_head_tasks.next, TASK_STRUCT_TASKS_OFFSET);
        get_linux_task_struct_info(task_struct, &pos);
    }
    //printf("%ld processes\n", count);
    memset(&pos, 0, sizeof(pos));
    return pos;
}

int find_cr3_in_task(linux_task_info task_info, u64 cr3)
{

    if (task_info.mm == 0)
    {
        phys_t active_mm_pgd_phys = task_info.active_mm_pgd_phys;
        
        if (cr3 == active_mm_pgd_phys)
        {
            printf("%s: FIND!!! task_name=%s, cr3:0x%llX, active_mm_pgd:0x%llX, active_mm_pgd_phys:0x%llX, active_mm:0x%llX\n", __func__, task_info.task_name, cr3, task_info.active_mm_pgd, task_info.active_mm_pgd_phys, task_info.active_mm);
            return 1;
        }
    }
    else
    {
        phys_t mm_pgd_phys = task_info.mm_pgd_phys;
        
        if (cr3 == mm_pgd_phys)
        {
            printf("%s: FIND!!! task_name=%s, cr3:0x%llX, mm_pgd:0x%llX, mm_pgd_phys:0x%llX, mm:0x%llX\n", __func__, task_info.task_name, cr3, task_info.mm_pgd, task_info.mm_pgd_phys, task_info.mm);
            return 1;
        }
    }

    return 0;
}

linux_task_info traverse_task_list_by_cr3(linux_task_info task_info, u64 cr3)
{

    linux_task_info pos;
    linux_virtaddr task_struct;
    ulong count = 0;
    int ret = 0;

    for (pos = task_info; pos.list_head_tasks.next != task_info.list_head_tasks.list_head_field;)
    {
        count++;
        
        //if(pos.parent_pid!=2)
        //{
            ret = find_cr3_in_task(pos, cr3);
            if(ret==1)
            {
                
                return pos;
            }
        //}
        task_struct = container_of(pos.list_head_tasks.next, TASK_STRUCT_TASKS_OFFSET);
        get_linux_task_struct_info(task_struct, &pos);
    }
    //printf("%ld processes\n", count);
    memset(&pos, 0, sizeof(pos));
    return pos;
}

linux_task_info find_cr3_in_task_list(u64 cr3)
{
    linux_task_info task_info;
    if(init_task != 0)
    {
        task_info = traverse_task_list_by_cr3(init_task_info, cr3);
    }
    return task_info;
}
//task-end



//files
int find_inode_in_task(linux_task_info task_info, linux_virtaddr inode)
{
    if (!is_linux_kernel_addr(task_info.fdtable))
    {
        printf("%s: Error: fdtable addr invalid!\n", __func__);
        return -1;
    }

    linux_virtaddr fdtable = task_info.fdtable;
    linux_virtaddr fd;
    linux_virtaddr file;
    linux_virtaddr inode_in_task;
    int i;

    read_virtaddr_data(fdtable + FDTABLE_FD_OFFSET, LINUX_ADDR_SIZE, &fd);

    for (i = 0; i < NR_OPEN_DEFAULT; i++)
    {
        read_virtaddr_data(fd, LINUX_ADDR_SIZE, &file); //get a struct file of struct files of task
        read_virtaddr_data(file + STRUCT_FILE_FINODE_OFFSET, LINUX_ADDR_SIZE, &inode_in_task);
        if (inode_in_task == inode)
        {
            return 1;
        }
        fd += LINUX_ADDR_SIZE;
    }
    return 0;
}

linux_task_info find_inode_in_task_list(linux_virtaddr inode)
{
    linux_task_info task_info;
    task_info = traverse_task_list_by_inode(init_task_info, inode);
    return task_info;
}
//files-end

u64 sg_page(linux_virtaddr page_link)
{

	return (page_link & ~(SG_CHAIN | SG_END));
}

u64 get_urbvirt_from_qtdm(struct ehci_qtd_meta *qtdm)
{
	linux_virtaddr urb_virt = *(linux_virtaddr *)(((unsigned char *)qtdm->qtd) + sizeof(struct ehci_qtd) + 28);
	return urb_virt;
}

u64 get_inode_from_qtdm(struct ehci_qtd_meta *qtdm)
{
	linux_virtaddr urb_virt;
	linux_virtaddr guesturbsg;
	linux_virtaddr page_link;
	linux_virtaddr page;
	linux_virtaddr address_space;
	linux_virtaddr inode;
    int ret = 0;

    urb_virt = get_urbvirt_from_qtdm(qtdm);

    ret = read_virtaddr_data(urb_virt + 112, LINUX_ADDR_SIZE, &guesturbsg);
    //printf("urb virt=0x%llX, urb->sg=0x%llX\n", urb_virt, guesturbsg);
    if(ret)
    {
        //printf("%s: urb->sg:0x%llX\n", __func__, guesturbsg);
        ret = read_virtaddr_data(guesturbsg, LINUX_ADDR_SIZE, &page_link);
        if (ret)
        {
            page = sg_page(page_link);
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
    //printf("page_link=0x%llX, struct page=0x%lX\n", page_link, page);

    ret = read_virtaddr_data(page + 24, LINUX_ADDR_SIZE, &address_space);
    //printf("address_space=0x%llX\n", address_space);

    if(ret)
    {
        ret = read_virtaddr_data(address_space, LINUX_ADDR_SIZE, &inode);
        //printf("inode addr=0x%llX\n", inode);
        if(ret)
        {
            return inode;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
    
}


int linux_add_to_dir_inode_extent_block(unsigned int block)
{
    if (block == 0)
    {
        return -1;
    }
    
    int i = 0;
    while (dir_inode_extent_block[i] != 0)
    {
        if (dir_inode_extent_block[i] == block)
        {
            return 1;
        }
        i++;
    }

    if (i < 500)
    {
        dir_inode_extent_block[i] = block;
        return 1;
    }
    else
    {
        printf("%s: dir_inode_extent_block[500] is full\n", __func__);
        return 0;
    }
}

int linux_traverse_dir_inode_extent_block(unsigned int block)
{
    int i = 0;
    while (dir_inode_extent_block[i] != 0)
    {
        if (dir_inode_extent_block[i] == block)
        {
            return 1;
        }
        i++;
    }
    return 0;
}

void linux_AddTag(struct cbw *cbw, unsigned int tag)
{
    // Add tag in usb_mscd.c -> usbmsc_cbw_parser();
    if (!cbw)
    {
        return;
    }

    switch (tag)
    {
    case RANSOMTAG:
        cbw->CBWCB[6] |= RANSOMTAG;
        //printf("%s: RansomTag added!!\n", __func__); 
        break;
    case RANSOMTAG_MFT:
        cbw->CBWCB[6] |= RANSOMTAG_MFT;
        //printf("%s: RANSOMTAG_MFT!!\n", __func__); 
        break;
    case RECOVERTAG:
        cbw->CBWCB[6] |= RECOVERTAG;
        break;
    default:
        break;
    }
}

/*
void analyse_qtd_linux(const struct mm_as *as, struct ehci_qtd_meta *qtdm)
{


     //get_init_task();   //run once
     //get_current_task();

    if(is_CBW(qtdm,NULL))
    {

        inode_in_qtd = 0;

        return;
    }
    *

    //linux_need_tag = RANSOMTAG;

    //start_pt_decode(5);


        //if (inode_in_qtd != 0)
        //{
        //    return;
        //}

        linux_task_info task_info;
        //qtdm->qtd = (struct ehci_qtd *)mapmem_gphys(qtdm->qtd_phys, sizeof(struct ehci_qtd) + 28, MAPMEM_WRITE);
        qtdm->qtd = (struct ehci_qtd *)mapmem_as(as, qtdm->qtd_phys, sizeof(struct ehci_qtd) + 28, MAPMEM_WRITE);
        inode_in_qtd = get_inode_from_qtdm(qtdm);

        if (inode_in_qtd != 0 && is_linux_kernel_addr(inode_in_qtd))
        {
            task_info = find_inode_in_task_list(inode_in_qtd);


        }


}
*/

int linux_is_inode_table_sector(unsigned int lba, int length)
{
    int i, count;
    unsigned int itable_blocks_count = (super_block.s_inode_size * super_block.s_inodes_per_group) / 4096;
    unsigned int lba_end = lba + length;
    count = sizeof(inode_table_start_sector) / sizeof(inode_table_start_sector[0]);
    for (i = 0; i < count; i++)
    {
        unsigned int inode_table_end_sector = EXT4_BLOCK_TO_SECTOR_START((SECTOR_TO_EXT4_BLOCK(inode_table_start_sector[i]) + itable_blocks_count));
        if (lba >= inode_table_start_sector[i] && lba <= inode_table_end_sector)
        {
            return 1;
        }
    }
    return 0;
}

int linux_is_inode_bitmap_blocks(unsigned int lba)
{
    int i, count;
    unsigned int block = SECTOR_TO_EXT4_BLOCK(lba);

    for (i = 0; i < 4; i++)
    {
        
        if (block==inode_bitmap_blocks[i])
        {
            return 1;
        }
    }
    return 0;
}

void linux_print_group_desc_info(void)
{
    for (size_t i = 0; i < min(group_total, loaded_group_desc_count); i++)
    {
        printf("<<<<<<<<<<<<<<<<<<< group_desc_%d >>>>>>>>>>>>>>>>>>>\n",i);
        printf("          Blocks bitmap block:  %d\n", group_desc[i].bg_block_bitmap_lo);
        printf("          Inodes bitmap block:  %d\n", group_desc[i].bg_inode_bitmap_lo);
        printf("           Inodes table block:  %d\n", group_desc[i].bg_inode_table_lo);
        printf("                EXT4_BG_flags:  %hd\n", group_desc[i].bg_flags);
        printf("<<<<<<<<<<<<<<<<<<<<<< end >>>>>>>>>>>>>>>>>>>>>>\n");
        printf("\n\n");
    }
}

void linux_print_superblock_info(void)
{
    printf("<<<<<<<<<<<<<<<<<<< superblock_info >>>>>>>>>>>>>>>>>>>\n");
    printf("          Magic signature:  %hX\n", super_block.s_magic);
    printf("             Inodes count:  %d\n", super_block.s_inodes_count);
    printf("             Blocks count:  %d\n", super_block.s_blocks_count_lo);
    printf("         First Data Block:  %d\n", super_block.s_first_data_block);
    printf("         Blocks per group:  %d\n", super_block.s_blocks_per_group);
    printf("         Inodes per group:  %d\n", super_block.s_inodes_per_group);
    printf("               Inode size:  %hd\n", super_block.s_inode_size);
    printf("<<<<<<<<<<<<<<<<<<<<<< end >>>>>>>>>>>>>>>>>>>>>>\n");
}

int linux_parse_ext4_group_desc(void *buf, int length)
{
    if (!buf)
    {
        return 0;
    }

    char *p = (char *)buf;
    int parsed_len = 0;
    int block_num = -1;
    int group_desc_num = -1;
    group_total = DIV_ROUND_UP(super_block.s_blocks_count_lo, super_block.s_blocks_per_group);

    printf("%s: group_total: %d\n", __func__, group_total);


    block_num = SECTOR_TO_EXT4_BLOCK(sector_index);

    if (block_num == 1 && loaded_group_desc_count < group_total)
    {
        do
        {
            memcpy(&group_desc[loaded_group_desc_count], p, EXT4_GROUP_DESC_SIZE);
            if (group_desc[loaded_group_desc_count].bg_flags == 0)
            {
                printf("%s: group_desc[%d] is invalid!\n", __func__, loaded_group_desc_count);
                return 0;
            }
            inode_table_start_sector[loaded_group_desc_count] = EXT4_BLOCK_TO_SECTOR_START(group_desc[loaded_group_desc_count].bg_inode_table_lo);
            printf("%s: group[%d] inode table start sector is [%d], block is [%d]\n", __func__, loaded_group_desc_count, inode_table_start_sector[loaded_group_desc_count], group_desc[loaded_group_desc_count].bg_inode_table_lo);
            inode_bitmap_blocks[loaded_group_desc_count] = group_desc[loaded_group_desc_count].bg_inode_bitmap_lo;
            loaded_group_desc_count++;
            p += EXT4_GROUP_DESC_SIZE;
            parsed_len += EXT4_GROUP_DESC_SIZE;
        } while (loaded_group_desc_count < group_total);
    }

    linux_print_group_desc_info();
    read_group_desc_metadata = false;
    group_desc_loaded = true;

    return 1;
}

int linux_parse_ext4_superblock(void *buf, int length)
{
    if (!buf || length < EXT4_SUPERBLOCK_SIZE)
    {
        return 0;
    }

    char *p = (char *)buf + EXT4_SUPERBLOCK_OFFSET_BYTE;

    memcpy(&super_block, (char *)p, EXT4_SUPERBLOCK_SIZE);

    if (super_block.s_magic != 0xEF53)
    {
        printf("%s: It is not a valid super block!\n", __func__);
    }

    linux_print_superblock_info();

    read_ext4_metadata = false;
    super_block_loaded = true;
    return 1;
}

int linux_ransomtag_storage_handle_sectors(struct storage_device *storage, struct storage_access *access, u8 *src, u8 *dst, int rw)
{
    count_t count = access->count;
    int sector_size = access->sector_size;
    
    if (count > 0 && dst != src)
    {
        // printf("%s: start copy. src: 0x%llX dst:0x%llX, len: %d \n", __func__, src, dst, count * sector_size); 
        memcpy(dst, src, count * sector_size);

        if (read_ext4_metadata && dst)
        {
            linux_parse_ext4_superblock(dst, count * sector_size);
        }
        if(read_group_desc_metadata && dst && super_block_loaded)
        {
            //printf("%s: to linux_parse_ext4_group_desc()...\n", __func__);
            linux_parse_ext4_group_desc(dst, count * sector_size);
        }

        //monitor the extent block in dir inodes during itable updating
        if (parse_inode_trigger == 1 && super_block_loaded && group_desc_loaded)
        {
            unsigned char *p = (unsigned char *)dst;
            for (size_t i = 0; i < count * sector_size / super_block.s_inode_size; i++)
            {
                //printf("%s: inode i=%d\n", __func__, i);
                if (*(u16 *)p & 0x4000)
                {
                    if (*(u16 *)(p + 40) == 0xF30A)  //extent_header magic
                    {
                        linux_add_to_dir_inode_extent_block(*(u32 *)(p + 60));
                        //printf("%s: extent_header magic: 0x%X\n", __func__, *(u16 *)(p + 40));
                        u64 addr = (*(u16 *)(p + 58)) << 32 | (*(u32 *)(p + 60));
                        //printf("%s: extent_start_addr: %d, u64 addr:%lld\n", __func__, *(u32 *)(p + 60), addr);
                    }
                }
                p += super_block.s_inode_size;
            }

            
        }

        // printf("%s: finish copy!\n\n", __func__); // 

        /*for entrop computing*/
        /*
        if (rw && dst)
        {
            buffer_byte_count(dst, count * sector_size);
        }
        */
        // printf("%s: count:%d  copied:%d  sector_size:%d)\n", __func__, count, count * sector_size, sector_size);
    }

    sector_index += count;

    return 0;
}

void linux_analyse_cbw(void *usb_msc_cbw)
{
    // printf("\n\n\n***New CBW***\n");
    int ret = 0;
    struct cbw *cbw = NULL;
    cbw = (struct cbw *)usb_msc_cbw;
    if (!cbw)
    {
        // printf("cbw INVALID!\n");
        return;
    }

    unsigned int lba;
    lba = (cbw->CBWCB[2] << 24) | (cbw->CBWCB[3] << 16) | (cbw->CBWCB[4] << 8) | (cbw->CBWCB[5] << 0);
    sector_index = lba;

    /*
    ret = 0;
    if ((file_lba_start != 0 && file_lba_end != 0) && (lba > file_lba_end || lba < file_lba_start) && lba != 0)
    {
        if (!win_traverse_process_blacklist(ProcessForEntropyCalc) && !traverse_entropy_excluded_process_list(ProcessForEntropyCalc))
        {
            ret = entropy_calc();
        }

        file_lba_start = file_lba_end = 0;

        if (ret == HIGH_ENTROPY)
        {
            //int ret2 = 0;
            //ret2 = win_add_to_high_entropylist(ProcessForEntropyCalc);
           // if(ret2==1)
            //{
                win_add_to_process_blacklist(ProcessForEntropyCalc);
                win_del_from_patternlist(ProcessForEntropyCalc);
                printf("%s: [%s] write high entropy, add to blakclist!\n", __func__, ProcessForEntropyCalc);
            //}
        }
    }
    */
    switch (cbw->CBWCB[0])
    {
    case 0x28: /* READ(10) */
        //printf("%s: Read: lba:%d, block:%d, sector:%d\n", __func__, lba, SECTOR_TO_EXT4_BLOCK(lba), EXT4_BLOCK_TO_SECTOR_START(SECTOR_TO_EXT4_BLOCK(lba)));

        if (lba == EXT4_SUPERBLOCK_LBA)
        {
            if (cbw->dCBWDataTransferLength == 4096 && !super_block_loaded)
            {
                read_ext4_metadata = true;
            }
            else if (cbw->dCBWDataTransferLength >= 4096 && !super_block_loaded && !group_desc_loaded)
            {
                read_ext4_metadata = true;
                read_group_desc_metadata = true;
            }
        }
        else if (((lba == EXT4_BLOCK_TO_SECTOR_START(SECTOR_TO_EXT4_BLOCK(EXT4_SUPERBLOCK_LBA) + 1))) && super_block_loaded && !group_desc_loaded)
        {
            read_group_desc_metadata = true;
        }

        if (linux_is_inode_table_sector(lba, cbw->dCBWDataTransferLength))
        {
            //printf("%s: inode table (lba[%d], len[%d]) is loaded, parsing inodes for dir inodes!\n", __func__, lba, cbw->dCBWDataTransferLength);
            parse_inode_trigger = 1;
            itable_transfer_length = cbw->dCBWDataTransferLength;
        }

        // to recognize the recovery process for mft start signal: read 8999 -> 0 -> 9000
        if (lba == EXT4_BLOCK_TO_SECTOR_START(120000))
        {
            trigger_mfttag_start = 1;
            //printf("%s: trigger start: trigger_mfttag_start == %d\n", __func__, trigger_mfttag_start);
            break;
        }

        // add mfttag for read out inodes
        if (trigger_mfttag_start == 1 && lba != EXT4_BLOCK_TO_SECTOR_START(120000) && 
                lba != EXT4_BLOCK_TO_SECTOR_START(120001) && lba != EXT4_BLOCK_TO_SECTOR_START(120002) 
                    && lba != EXT4_BLOCK_TO_SECTOR_START(120003))
        {

            linux_AddTag(cbw, RANSOMTAG_MFT);
            printf("%s: Read: block[%d], linux_AddTag(cbw, RANSOMTAG_MFT).....\n", __func__, SECTOR_TO_EXT4_BLOCK(lba));
            break;
        }

        // end mfttag signal
        if (lba == EXT4_BLOCK_TO_SECTOR_START(120001) && trigger_mfttag_start == 1)
        {

            trigger_mfttag_start = 0;
            //printf("%s: trigger end: trigger_mfttag_start == %d\n", __func__, trigger_mfttag_start);
            break;
        }
        /*
        if (trigger_mfttag_end==3)
        {

            //printf("%s: this round of reading inode is end\n", __func__);
            trigger_mfttag_start = 0;
            trigger_mfttag_end = 0;
        }*/

        // to recognize the recovery process for recovery start signal: read 8999 -> 0 -> 9000
        if (lba == EXT4_BLOCK_TO_SECTOR_START(120002))
        {
            trigger_recovertag_start = 1;
            // printf("%s: trigger_recovertag_start++ == %d\n", __func__, trigger_recovertag_start);
            break;
        }

        // add recovertag for read out inodes
        if (trigger_recovertag_start == 1 && lba != EXT4_BLOCK_TO_SECTOR_START(120000) && 
                lba != EXT4_BLOCK_TO_SECTOR_START(120001) && lba != EXT4_BLOCK_TO_SECTOR_START(120002) 
                    && lba != EXT4_BLOCK_TO_SECTOR_START(120003))
        {

            linux_AddTag(cbw, RECOVERTAG);
            printf("%s: block[%d], reading retained data ....\n", __func__, SECTOR_TO_EXT4_BLOCK(lba));
            break;
        }

        // end recovertag signal
        if (lba == EXT4_BLOCK_TO_SECTOR_START(120003) && trigger_recovertag_start == 1)
        {

            trigger_recovertag_start = 0;
            // printf("%s: trigger_recovertag_end++ == %d\n", __func__, trigger_recovertag_end);
            break;
        }
        /*
        if (trigger_recovertag_end==3)
        {

            //printf("%s: recover is end\n", __func__);
            trigger_recovertag_start = 0;
            trigger_recovertag_end = 0;
        }
        */

        //printf("%s: read break\n", __func__);
        break;
    case 0x2a: /* WRITE(10) */
        //printf("%s: Write: lba:%d, block:%d, sector:%d\n", __func__, lba, SECTOR_TO_EXT4_BLOCK(lba), EXT4_BLOCK_TO_SECTOR_START(SECTOR_TO_EXT4_BLOCK(lba)));

        if (linux_is_inode_table_sector(lba, cbw->dCBWDataTransferLength))
        {
            printf("%s: Write: Block [%d] (lba [%d]) linux_AddTag(cbw, RANSOMTAG_MFT)\n", __func__, SECTOR_TO_EXT4_BLOCK(lba), lba);
            linux_AddTag(cbw, RANSOMTAG_MFT);
            parse_inode_trigger = 1;
            itable_transfer_length = cbw->dCBWDataTransferLength;
        }
        else if(linux_is_inode_bitmap_blocks(lba))
        {
            printf("%s:Block [%d] (lba [%d]) is inode_bitmap, linux_AddTag(cbw, RANSOMTAG_MFT)\n", __func__, SECTOR_TO_EXT4_BLOCK(lba), lba);
            linux_AddTag(cbw, RANSOMTAG_MFT);
        }
        else if (linux_traverse_dir_inode_extent_block(SECTOR_TO_EXT4_BLOCK(lba)) == 1)
        {
            printf("%s:Block [%d] (lba [%d]) is dir extent block, linux_AddTag(cbw, RANSOMTAG_MFT)\n", __func__, SECTOR_TO_EXT4_BLOCK(lba), lba);
            linux_AddTag(cbw, RANSOMTAG_MFT);
        }
        else
        {   
            //performing detection algorithm
            linux_AddTag(cbw, RANSOMTAG);
        }

        break;

    default:
        break;
    }
}

/*************************  Linux End  ***************************************************************************************************/











/*************************  Windows Begin  ***************************************************************************************************/

void win_ransomtag_init(void)
{
    memset(patternlist, 0, sizeof(win_ransom_pattern) * 100);
}

int is_win_kernel_addr(Win_virtaddr addr)
{
    int ret = 0;
    if (addr >= WIN_KERNEL_ADDR_SPACE_START && addr <= WIN_KERNEL_ADDR_SPACE_END)
    {
        ret = 1;
    }
    else
    {
        // printf("%s virt_addr[0x%llX] is not a kernel addr!!!\n", __func__, addr);
        ret = 0;
    }
    return ret;
}

int win_read_virtaddr_data(Win_virtaddr virt_addr, int read_len, void *data)
{

    if (!is_win_kernel_addr(virt_addr) || !data)
    {
        return 0;
    }

    // unsigned char *data_array = (unsigned char *)data;
    for (int i = 0; i < read_len; i++)
    {
        read_linearaddr_b(virt_addr, (unsigned char *)data);
        virt_addr++;
        (unsigned char *)data++;
    }
    return 1;
}

Win_virtaddr win_container_of(Win_virtaddr list_entry_field, u64 offset)
{
    return list_entry_field - offset;
}

Win_virtaddr win_get_KPCR(void)
{
    Win_virtaddr gs_base;
    get_msr_gs_base(&gs_base);
    return gs_base;
}

Win_virtaddr win_get_KPRCB(void)
{
    Win_virtaddr KPRCB;
    Win_virtaddr KPCR = win_get_KPCR();
    win_read_virtaddr_data(KPCR + STRUCT_KPCR_KPRCB_OFFSET, 8, &KPRCB);
    return KPRCB;
}

u64 win_get_cr3(void)
{
    u64 cr3;
    get_current_cr3_reg(&cr3);
    return cr3;
}

Win_virtaddr win_get_CurrentThread(void)
{
    Win_virtaddr CurrentThread;
    Win_virtaddr KPRCB = win_get_KPRCB();
    win_read_virtaddr_data(KPRCB + STRUCT_KPRCB_CURRENT_THREAD_OFFSET, 8, &CurrentThread);
    return CurrentThread;
}

int win_get_CurrentProcessInfo(win_process_info *win_process_info)
{
    if (!win_process_info)
    {
        printf("%s: win_process_info NULL\n", __func__);
        return 0;
    }

    Win_virtaddr CurrentThread = win_get_CurrentThread();
    if (!is_win_kernel_addr(CurrentThread))
    {
        printf("%s: CurrentThread INVALID\n", __func__);
        return 0;
    }

    Win_virtaddr CurrentProcess;
    u64 cr3;
    u64 DirectoryTableBase;
    win_read_virtaddr_data(CurrentThread + STRUCT_ETHREAD_PROCESS_OFFSET, 8, &CurrentProcess);
    win_read_virtaddr_data(CurrentThread + STRUCT_ETHREAD_IRPLIST_OFFSET, 8, &win_process_info->IrpList.Flink);
    win_read_virtaddr_data(CurrentThread + STRUCT_ETHREAD_IRPLIST_OFFSET + 8, 8, &win_process_info->IrpList.Blink);
    get_current_cr3_reg(&cr3);
    win_read_virtaddr_data(CurrentProcess + STRUCT_EPROCESS_DIRECTORY_TABLE_BASE_OFFSET, 8, &DirectoryTableBase);

    win_read_virtaddr_data(CurrentProcess + STRUCT_EPROCESS_IMAGE_FILE_NAME_OFFSET, 16, win_process_info->ProcessName);
    win_process_info->EThread = CurrentThread;
    win_process_info->EProcess = CurrentProcess;
    win_process_info->cr3 = cr3;
    win_process_info->DirectoryTableBase = DirectoryTableBase;
    win_process_info->IrpList.list_entry_field = CurrentThread + STRUCT_ETHREAD_IRPLIST_OFFSET;
    for (size_t i = 0; i < 5; i++)
    {
        win_process_info->IrpInfo[i].used = false;
    }
    return 1;
}

void win_print_process_info(win_process_info *win_process_info)
{
    if (win_process_info)
    {
        printf("<<<<<<<<<<<<<<<<<<< win_process_info >>>>>>>>>>>>>>>>>>>\n");
        printf("             ProcessName:  %s\n", win_process_info->ProcessName);
        printf("                EProcess:  0x%llX\n", win_process_info->EProcess);
        printf("                 EThread:  0x%llX\n", win_process_info->EThread);
        printf("                 IrpList:  0x%llX\n", win_process_info->IrpList.list_entry_field);
        printf("      DirectoryTableBase:  0x%llX\n", win_process_info->DirectoryTableBase);
        printf("                     cr3:  0x%llX\n", win_process_info->cr3);
        printf("<<<<<<<<<<<<<<<<<<<<<<<<< end >>>>>>>>>>>>>>>>>>>>>>>>>\n");
    }
}

int traverse_excluded_process_list(unsigned char *ProcessName)
{
    int i, count;
    count = sizeof(win_excluded_processes) / sizeof(win_excluded_processes[0]);
    for (i = 0; i < count; i++)
    {
        if (mem_find_str(ProcessName, 14, win_excluded_processes[i]))
        {
            return 1;
        }
    }
    return 0;
}

int traverse_entropy_excluded_process_list(unsigned char *ProcessName)
{
    int i, count;
    count = sizeof(win_entropy_excluded_processes) / sizeof(win_entropy_excluded_processes[0]);
    for (i = 0; i < count; i++)
    {
        if (mem_find_str(ProcessName, 14, win_entropy_excluded_processes[i]))
        {
            return 1;
        }
    }
    return 0;
}

int traverse_IrpList(win_process_info *CurrentProcessInfo, unsigned int lba)
{
    if (!is_win_kernel_addr(CurrentProcessInfo->IrpList.list_entry_field))
    {
        // printf("%s: Irplist is INVALID\n", __func__);
        return 0;
    }

    Win_virtaddr Irp;
    Win_virtaddr ListEntryIndex;
    Win_virtaddr Flink;
    int IrpCount = 0;
    for (ListEntryIndex = CurrentProcessInfo->IrpList.Flink; ListEntryIndex != CurrentProcessInfo->IrpList.list_entry_field; ListEntryIndex = Flink)
    {
        if (!is_win_kernel_addr(ListEntryIndex))
        {
            break;
        }
        Irp = win_container_of(ListEntryIndex, STRUCT_IRP_THREAD_LIST_ENTRY_OFFSET);
        // printf("IrpList: 0x%llX\n", Irp);

        if (!is_win_kernel_addr(Irp))
        {
            // printf("%s: Irp is INVALID\n", __func__);
            break;
        }

        CurrentProcessInfo->IrpInfo[IrpCount].Irp = Irp;
        win_get_IrpInfo(&CurrentProcessInfo->IrpInfo[IrpCount], lba);
        // win_print_irp_info(&IrpInfo);
        IrpCount++;
        win_read_virtaddr_data(ListEntryIndex, 8, &Flink);
        if (IrpCount == 4)
            break;
    }

    return 1;
}

void win_get_FileObjectInfo(win_file_object_info *win_file_object_info)
{
    win_read_virtaddr_data(win_file_object_info->FileObject + STRUCT_FILE_OBJECT_FILENAME_OFFSET, 2, &win_file_object_info->FileName.Length);
    win_read_virtaddr_data(win_file_object_info->FileObject + STRUCT_FILE_OBJECT_FILENAME_OFFSET + STRUCT_UNICODE_STRING_BUFFER_OFFSET, 8, &win_file_object_info->FileName.Buffer);
    win_read_virtaddr_data(win_file_object_info->FileName.Buffer, win_file_object_info->FileName.Length, win_file_object_info->FileName.FileNameStr);
}

int win_is_MFT_or_Bitmap(unsigned char *FileNameStr, unsigned int lba)
{
    if (memcmp(FileNameStr, NTFS_MetaFile_MFT, 9) == 0)
    {
        return IS_MFT;
    }
    else if (lba == BITMAP_LBA || (memcmp(FileNameStr, NTFS_MetaFile_Bitmap, 15) == 0))
    {
        return IS_BITMAP;
    }
    return 0;
}

void win_get_IrpInfo(win_irp_info *win_irp_info, unsigned int lba)
{
    Win_virtaddr Irp = win_irp_info->Irp;

    if (!is_win_kernel_addr(Irp))
    {
        // printf("%s: Irp is INVALID\n", __func__);
        return;
    }

    win_read_virtaddr_data(Irp + STRUCT_IRP_STACK_COUNT_OFFSET, 1, &win_irp_info->StackCount);
    win_read_virtaddr_data(Irp + STRUCT_IRP_CURRENT_LOCATION_OFFSET, 1, &win_irp_info->CurrentLocation);
    win_read_virtaddr_data(Irp + IRP_IO_STACK_LOCATION_OFFSET, 8, &win_irp_info->CurrentStackLocation);
    win_irp_info->TopStackLocation = win_irp_info->CurrentStackLocation + ((win_irp_info->StackCount - win_irp_info->CurrentLocation) * SIZEOF_STRUCT_IO_STACK_LOCATION);
    win_read_virtaddr_data(win_irp_info->TopStackLocation, 1, &win_irp_info->MajorFunction);
    win_read_virtaddr_data(Irp + IRP_ORIGINAL_FILE_OBJECT_OFFSET, 8, &win_irp_info->FileObjectInfo.FileObject);

    if (!is_win_kernel_addr(win_irp_info->FileObjectInfo.FileObject))
    {
        // printf("%s: FileObject is INVALID\n", __func__);
        return;
    }

    win_get_FileObjectInfo(&win_irp_info->FileObjectInfo);

    if ((win_irp_info->MajorFunction == IRP_MJ_READ) || (win_irp_info->MajorFunction == IRP_MJ_WRITE))
    {
        win_read_virtaddr_data(win_irp_info->TopStackLocation + STRUCT_IO_STACK_LOCATION_BYTEOFFSET_OFFSET, 8, &win_irp_info->FileObjectInfo.ByteOffset);
        win_read_virtaddr_data(win_irp_info->TopStackLocation + STRUCT_IO_STACK_LOCATION_LENGTH_OFFSET, 8, &win_irp_info->ReadWriteLength);
        // printf("%s: ByteOffset: %lld\n", __func__, win_irp_info->FileObjectInfo.ByteOffset);
    }
    else
    {
        win_irp_info->FileObjectInfo.ByteOffset = 0;
        win_irp_info->ReadWriteLength = 0;
    }

    win_irp_info->FileObjectInfo.is_MFT = false;
    win_irp_info->FileObjectInfo.is_Bitmap = false;
    win_irp_info->used = true;

    switch (win_is_MFT_or_Bitmap(win_irp_info->FileObjectInfo.FileName.FileNameStr, lba))
    {
    case IS_MFT:
        win_irp_info->FileObjectInfo.is_MFT = true;
        // printf("Find $MFT!!!!!!!!!!!!!!\n");
        break;
    case IS_BITMAP:
        win_irp_info->FileObjectInfo.is_Bitmap = true;
        printf("Find $Bitmap!!!!!!!!!!!!!! lba: %d\n", lba);
        if ((bitmap_inited == true) && (win_irp_info->MajorFunction == IRP_MJ_WRITE))
        {
            bitmap_check_trigger = true;
        }

        bitmap_lba = lba;
        bitmap_offset = win_irp_info->FileObjectInfo.ByteOffset;
        break;
    default:
        break;
    }
}

void win_print_irp_info(win_process_info *CurrentProcessInfo)
{
    win_irp_info *IrpInfo = CurrentProcessInfo->IrpInfo;
    int IrpCount;
    for (IrpCount = 0; IrpCount < 5; IrpCount++)
    {
        if (IrpInfo[IrpCount].used == false)
        {
            break;
        }

        printf("Irp %d:\n", IrpCount);
        printf("<<<<<<<<<<<<<<<<<<< win_irp_info >>>>>>>>>>>>>>>>>>>\n");
        printf("                     Irp:  0x%llX\n", IrpInfo[IrpCount].Irp);
        printf("           MajorFunction:  0x%X\n", IrpInfo[IrpCount].MajorFunction);
        printf("              StackCount:  0x%X\n", IrpInfo[IrpCount].StackCount);
        printf("         CurrentLocation:  0x%X\n", IrpInfo[IrpCount].CurrentLocation);
        printf("    CurrentStackLocation:  0x%llX\n", IrpInfo[IrpCount].CurrentStackLocation);
        printf("        TopStackLocation:  0x%llX\n", IrpInfo[IrpCount].TopStackLocation);
        printf("              FileObject:  0x%llX\n", IrpInfo[IrpCount].FileObjectInfo.FileObject);
        printf("           FileNameLenth:  %d\n", IrpInfo[IrpCount].FileObjectInfo.FileName.Length);
        printf("         ReadWriteLength:  %lld\n", IrpInfo[IrpCount].ReadWriteLength);

        int i;
        printf("FileName: ");
        for (i = 0; i < IrpInfo[IrpCount].FileObjectInfo.FileName.Length; i++)
        {
            printf("%2X ", IrpInfo->FileObjectInfo.FileName.FileNameStr[i]);
            if (i >= 512)
            {
                break;
            }
        }
        printf("\n");
        printf("<<<<<<<<<<<<<<<<<<<<<<<<< end >>>>>>>>>>>>>>>>>>>>>>>>>\n");
        printf("\n");
    }
}

/*
void analyse_qtd_windows(struct ehci_qtd_meta *qtdm)
{
    struct cbw_info cbw_info;
    if (is_CBW(qtdm, &cbw_info))
    {
        parse_CBW(&cbw_info);
        win_process_info CurrentProcessInfo;
        win_get_CurrentProcessInfo(&CurrentProcessInfo);

        if (!traverse_excluded_process_list(CurrentProcessInfo.ProcessName))
        {
            win_print_process_info(&CurrentProcessInfo);
            //traverse_IrpList(&CurrentProcessInfo);
        }
    }
}
*/

int win_AddToSectorPageMap(u32 sector, u32 length)
{
    // int Page = sector / 8;
    // int PageCount = (length - 1) / 8 + 1;

    // KdPrint((DRIVERNAME "%s: Sector=%u, length=%u\n", __func__, sector, length));

    for (u32 i = 0; i < length; i++)
    {
        SetBitmap(sector);
        sector++;
    }

    return 0;
}

void win_load_bitmap(void)
{
    if (bitmap_inited == true)
    {
        printf("bitmap has already been loaded!\n");
        return;
    }

    bitmap_load_trigger = true;
}

int win_AddToBitmap(unsigned char *buffer, unsigned long length)
{
    unsigned long i;
    unsigned long long element = bitmap_offset;

    for (i = 0; i < length; i++)
    {
        bitmap[element] = buffer[i];
        element++;
    }
    printf("%s: Add to $Bitmap! offset:%lld, len:%d\n", __func__, bitmap_offset, length);

    return 0;
}

int win_CheckBitmap(unsigned char *buffer, unsigned long length)
{
    if (!buffer)
    {
        // printf("%s: buffer is NULL\n", __func__);
        return 0;
    }

    u32 i, j;
    unsigned long long element = bitmap_offset;
    u32 RunlistStartCluster = 0;
    u32 RunlistLength = 0;
    u32 LengthCounter = 0;
    int StartFlag = 1;
    int ContinueFlag = 0;
    unsigned char CompareResult[4096] = {0};
    if (length > 4096)
    {
        printf("%s: Err! length [%d] > 4096\n", __func__, length);
    }

    for (i = 0; i < length; i++)
    {
        if (bitmap_compare_trigger == true)
        {
            CompareResult[i] = (~buffer[i]) & bitmap[element];
        }
        bitmap[element] = buffer[i];
        element++;
    }

    if (bitmap_compare_trigger == true)
    {
        for (j = 0; j < (length * 8); j++)
        {
            if (CompareResult[j >> SHIFT] & (1 << (j & MASK)))
            {
                if (StartFlag == 1)
                {
                    RunlistStartCluster = j + bitmap_offset * 8;
                    LengthCounter++;
                    StartFlag = 0;
                    ContinueFlag = 1;
                }
                else
                {
                    LengthCounter++;
                }
            }
            else if (ContinueFlag == 1)
            {
                RunlistLength = LengthCounter;
                LengthCounter = 0;
                StartFlag = 1;
                ContinueFlag = 0;
                win_AddToSectorPageMap(RunlistStartCluster * 8 + 2048, RunlistLength * 8);
            }
        }
    }

    return 0;
}

void win_AddTag(struct cbw *cbw, unsigned int tag)
{
    if (!cbw)
    {
        return;
    }

    switch (tag)
    {
    case RANSOMTAG:
        cbw->CBWCB[6] |= RANSOMTAG;
        break;
    case RANSOMTAG_MFT:
        cbw->CBWCB[6] |= RANSOMTAG_MFT;
        break;
    case RECOVERTAG:
        cbw->CBWCB[6] |= RECOVERTAG;
        break;
    default:
        break;
    }
}

int win_SearchSectorMap(struct cbw *cbw)
{
    unsigned int lba;
    unsigned long length;
    int ret = 0;
    int i;

    lba = (cbw->CBWCB[2] << 24) | (cbw->CBWCB[3] << 16) | (cbw->CBWCB[4] << 8) | (cbw->CBWCB[5] << 0);
    length = (unsigned long)cbw->dCBWDataTransferLength;

    for (i = 0; i < length; i++)
    {
        ret = TestBitmap(lba);
        if (ret)
        {
            return 1;
        }
        lba++;
    }
    return 0;
}

int win_traverse_process_blacklist(unsigned char *ProcessName)
{
    int i = 0;
    while (win_processe_blacklist[i][0] != '\0')
    {
        if (mem_find_str(ProcessName, 14, win_processe_blacklist[i]))
        {
            return 1;
        }
        i++;
    }
    return 0;
}

int win_add_to_process_blacklist(unsigned char *ProcessName)
{
    if (traverse_excluded_process_list(ProcessName))
    {
        return -1;
    }

    int i = 0;
    while (win_processe_blacklist[i][0] != '\0')
    {
        if (mem_find_str(ProcessName, 14, win_processe_blacklist[i]))
        {
            printf("%s: [%s] has already in the blacklist!\n", __func__, ProcessName);
            return 1;
        }
        i++;
    }

    if (i < 100)
    {
        memcpy(win_processe_blacklist[i], ProcessName, 15);
        return 1;
    }
    else
    {
        return 0;
    }
}

int win_add_to_high_entropylist(unsigned char *ProcessName)
{
    /*
    if (traverse_excluded_process_list(ProcessName))
    {
        return -1;
    }
    */

    int i = 0;
    while (win_high_entropylist[i][0] != '\0')
    {
        if (mem_find_str(ProcessName, 14, win_high_entropylist[i]))
        {
            printf("%s: [%s] has already in the high_entropylist!\n", __func__, ProcessName);
            return 1;
        }
        i++;
    }

    if (i < 100)
    {
        memcpy(win_high_entropylist[i], ProcessName, 15);
        return 2;
    }
    else
    {
        memset(win_high_entropylist, 0, 100 * 15);
        memcpy(win_high_entropylist[0], ProcessName, 15);
        return 3;
    }
}

int win_traverse_high_entropylist(unsigned char *ProcessName)
{
    int i = 0;
    while (win_high_entropylist[i][0] != '\0')
    {
        if (mem_find_str(ProcessName, 14, win_high_entropylist[i]))
        {
            return 1;
        }
        i++;
    }
    return 0;
}

int win_traverse_patternlist(unsigned char *ProcessName, unsigned char *filename, unsigned int filename_length, int FileActivity)
{
    /*
    if(memcmp(filename,"\\",1)==0)
    {
        printf("%s: filename has a \\ \n", __func__);
        filename += 2;
        filename_length -= 2;
    }*/

    int i = 0;
    while (patternlist[i].used == true && i < 100)
    {

        switch (FileActivity)
        {
        case READFILE:
            if (memcmp(patternlist[i].ReadFileName, filename, filename_length) == 0)
            {
                return i;
            }
            break;
        case WRITEFILE:
            if (memcmp(patternlist[i].WriteFileName, filename, filename_length) == 0)
            {
                return i;
            }
            break;
        case DELETEFILE:
            if (memcmp(patternlist[i].DeleteFileName, filename, filename_length) == 0)
            {
                return i;
            }
            break;
        default:
            break;
        }

        i++;
    }
    return -1;
}

int win_add_to_patternlist(unsigned char *ProcessName, unsigned char *filename, unsigned int filename_length, int FileActivity)
{
    if (win_traverse_process_blacklist(ProcessName))
    {
        return 0;
    }
    /*
    if(memcmp(filename,"\\",1)==0)
    {
        printf("%s: filename has a \\ ,filename_length: %d\n", __func__, filename_length);
        filename += 2;
        filename_length -= 2;
        if(filename_length==0)
        {
            return 0;
        }
    }
    */
    if (filename_length < 2)
    {
        return 0;
    }

    if (filename_length > 512)
    {
        filename_length = 512;
    }

    int filename_length_new = 0;
    unsigned char *filename_tmp = filename + (filename_length - 2);
    while (memcmp(filename_tmp, "\\", 1) != 0)
    {
        filename_length_new += 2;
        filename_tmp -= 2;
    }
    filename = filename_tmp + 2;
    filename_length = filename_length_new;
    if (filename_length == 0)
    {
        return 0;
    }

    int i;
    int used_count = 0;
    size_t j;
    for (i = 0; i < 100; i++)
    {
        if (patternlist[i].used != true)
        {
            continue;
        }

        used_count++;

        if (FileActivity == READFILE)
        {
            if ((memcmp(patternlist[i].ReadFileName, filename, filename_length) == 0) && (mem_find_str(patternlist[i].ProcessName, 14, ProcessName)))
            {
                /*
                printf("%s: [%s]: filename_length: %d, file [", __func__, ProcessName, filename_length);
                for (j = 0; j < filename_length; j++)
                {
                    printf("%c", filename[j]);
                    if (j >= 512)
                    {
                        break;
                    }
                }
                printf("] has already in patternlist[%d]! FileActivity: %d\n", i,FileActivity);
                */
                return 1;
            }
        }
        else if (FileActivity == WRITEFILE)
        {
            if (mem_find_str(ProcessName, 14, ProcessForPattern) && mem_find_str(patternlist[i].ProcessName, 14, ProcessName) && filename_length <= 42 && (memcmp(NTFS_MetaFile_ConvertToNonresident, filename, filename_length) == 0))
            {

                if (memcmp(NTFS_MetaFile_ConvertToNonresident, patternlist[i].WriteFileName, 42) == 0)
                {
                    return 1;
                }
                else if (patternlist[i].WriteFileName[0] == '\0')
                { /*
                  printf("%s: [%s]: >>>>>filename_length: %d, patternlist[%d].WriteFileName [", __func__, ProcessName, filename_length, i);
                  for (j = 0; j < filename_length; j++)
                  {
                      printf("%c", filename[j]);
                      if (j >= 512)
                      {
                          break;
                      }
                  }
                  printf("]<<<<<>>>>> FileActivity: %d\n", FileActivity);
                  */
                    memcpy(patternlist[i].WriteFileName, filename, filename_length);
                    return 1;
                }
            }

            if (memcmp(patternlist[i].ReadFileName, filename, filename_length) == 0)
            {
                if (memcmp(patternlist[i].WriteFileName, filename, filename_length) == 0)
                {
                    /*
                    printf("%s: [%s]: filename_length: %d, file [", __func__, ProcessName, filename_length);
                    for (j = 0; j < filename_length; j++)
                    {
                        printf("%c", filename[j]);
                        if (j >= 512)
                        {
                            break;
                        }
                    }
                    printf("] has already in patternlist! FileActivity: %d\n", FileActivity);
                    */
                    return 1;
                }
                else if (patternlist[i].WriteFileName[0] == '\0')
                {
                    memcpy(patternlist[i].WriteFileName, filename, filename_length);
                    return 1;
                }
            }
        }
    }

    if (used_count >= 100)
    {
        memset(patternlist, 0, sizeof(win_ransom_pattern) * 100);
    }

    if (FileActivity != READFILE)
    {
        return 0;
    }

    i = 0;
    while (i < 100)
    {
        if (patternlist[i].used != true)
        {
            break;
        }
        i++;
    }

    memcpy(patternlist[i].ProcessName, ProcessName, 15);
    patternlist[i].used = true;
    memcpy(patternlist[i].ReadFileName, filename, filename_length);

    /*
    printf("%s: add Process:[%s]/ File [\n", __func__, ProcessName);
    for (size_t k = 0; k < filename_length; k++)
    {
        printf("%c", filename[k]);
        if (k >= 512)
        {
            break;
        }
    }
    printf("] to patternlist[%d]! FileActivity: %d, filename_length: %d\n", i,FileActivity,filename_length);
    */

    return 1;
}

int win_del_from_patternlist(unsigned char *ProcessName)
{

    int i;
    for (i = 0; i < 100; i++)
    {
        if (mem_find_str(patternlist[i].ProcessName, 14, ProcessName))
        {
            memset(&patternlist[i], 0, sizeof(win_ransom_pattern));
        }
    }

    return 1;
}

void win_Handle_Recovery(struct cbw *cbw)
{
    unsigned int sector;
    unsigned long length;

    sector = (cbw->CBWCB[2] << 24) | (cbw->CBWCB[3] << 16) | (cbw->CBWCB[4] << 8) | (cbw->CBWCB[5] << 0);
    length = (unsigned long)cbw->dCBWDataTransferLength;

    win_process_info CurrentProcessInfo;
    win_get_CurrentProcessInfo(&CurrentProcessInfo);

    if (!is_win_kernel_addr(CurrentProcessInfo.EThread))
    {
        printf("%s: CurrentProcessInfo.EThread is INVALID\n", __func__);
        return;
    }

    if (mem_find_str(CurrentProcessInfo.ProcessName, 11, "scanmft"))
    {
        if (sector == 2048)
        {
            sector2048count++;
        }
        if (sector2048count == 2 && sector != 2048)
        {
            win_AddTag(cbw, RANSOMTAG_MFT);
            printf("%s: Add scanmft tag......\n", __func__);
        }
        else if (sector2048count == 3)
        {
            sector2048count = 0;
        }
    }
    else if (mem_find_str(CurrentProcessInfo.ProcessName, 13, "ransomrecover"))
    {
        win_AddTag(cbw, RECOVERTAG);
        printf("%s: Add data recover tag......\n", __func__);
    }
}

int ParseMFT(void *buf, unsigned int length)
{
    if (!buf)
    {
        return 0;
    }

    int MFT_Count = length / 1024;
    unsigned char *index = (unsigned char *)buf;

    for (int j = 0; j < MFT_Count; j++)
    {
        pFILE_RECORD fileRecord = (pFILE_RECORD)index;

        PATTRIBUTE attribute = (PATTRIBUTE)((unsigned char *)fileRecord + fileRecord->AttributeOffset);
        PFILENAME_ATTRIBUTE fileName_Attribute = NULL;
        PRESIDENT_ATTRIBUTE fileName_Attribute_Header = NULL;

        if (fileRecord->Ntfs.Type != 0x454C4946)
        {
            index += 1024;
            continue;
        }
        printf("%s: MFT Flags: %d, MFTRecordNumber: %d\n", __func__, fileRecord->Flags, fileRecord->MFTRecordNumber);
        while (((unsigned char *)attribute - (unsigned char *)fileRecord) < MFT_FILE_SIZE)
        {

            if (attribute->AttributeType == 0x30)
            {
                fileName_Attribute_Header = (PRESIDENT_ATTRIBUTE)attribute;
                fileName_Attribute = (PFILENAME_ATTRIBUTE)((unsigned char *)fileName_Attribute_Header + fileName_Attribute_Header->ValueOffset);
                unsigned char *filename = (unsigned char *)fileName_Attribute->Name;
                unsigned char filename_length = fileName_Attribute->NameLength * sizeof(short);

                printf("%s:***filename: ", __func__);
                for (size_t l = 0; l < filename_length; l++)
                {
                    printf("%c", filename[l]);
                    if (l >= 512)
                    {
                        break;
                    }
                }
                printf("\n");

                if (fileRecord->Flags == 0)
                {
                    int x;
                    unsigned char ProcessName[15] = {0};

                    for (x = 0; x < 100; x++)
                    {
                        if (patternlist[x].used != true)
                        {
                            continue;
                        }
                        /*
                        printf("%s: %d: file [", __func__,x);
                        for (size_t i = 0; i < filename_length; i++)
                        {
                            printf("%c", filename[i]);
                            if (i >= 512)
                            {
                                break;
                            }
                        }
                        printf("] ,filename_length: %d\n",filename_length);
                        */

                        /*
                        printf("%s: patternlist[%d].ReadFileName [", __func__,x);
                        for (size_t i = 0; i < filename_length; i++)
                        {
                            printf("%c", patternlist[x].ReadFileName[i]);
                            if (i >= 512)
                            {
                                break;
                            }
                        }
                        printf("] ,filename_length: %d\n",filename_length);
                        */

                        if (memcmp(patternlist[x].ReadFileName, filename, filename_length) == 0)
                        {
                            /*
                            printf("x: %d\n", x);
                            printf("%s: patternlist[%d].WriteFileName [", __func__, x);
                            for (size_t i = 0; i < filename_length; i++)
                            {
                                printf("%c", patternlist[x].WriteFileName[i]);
                                if (i >= 512)
                                {
                                    break;
                                }
                            }
                            printf("] ,filename_length: %d\n", filename_length);
                            */

                            if ((memcmp(patternlist[x].WriteFileName, filename, filename_length) == 0) ||
                                (memcmp(patternlist[x].WriteFileName, NTFS_MetaFile_ConvertToNonresident, 42) == 0))
                            {
                                memcpy(ProcessName, patternlist[x].ProcessName, 15);
                                if (win_traverse_high_entropylist(ProcessName))
                                {
                                    win_add_to_process_blacklist(ProcessName);
                                    win_del_from_patternlist(ProcessName);

                                    printf("%s:file [", __func__);
                                    for (size_t i = 0; i < filename_length; i++)
                                    {
                                        printf("%c", filename[i]);
                                        if (i >= 512)
                                        {
                                            break;
                                        }
                                    }
                                    printf("] is deleted! and [%s] has HIGH entropy write, add to blacklist!\n", ProcessName);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            else if (attribute->AttributeType == 0xFFFFFFFF)
            {
                break;
            }

            attribute = (PATTRIBUTE)((unsigned char *)attribute + attribute->RecordLength);
        }

        index += 1024;
    }

    return 0;
}

int win_ransomtag_storage_handle_sectors(struct storage_device *storage, struct storage_access *access, u8 *src, u8 *dst, int rw)
{
    count_t count = access->count;
    int sector_size = access->sector_size;
    int bitmap_count = 0; 
    if (count > 0 && dst != src)
    {

        // first load bitmap from ssd
        if ((bitmap_load_trigger == true) && (!rw) && src)
        {
            win_AddToBitmap(src, count * sector_size);
            printf("%s: bitmap loading... bitmap_count: %d, len:%ld, lba:%d\n", __func__, bitmap_count++, count * sector_size, access->lba);
            bitmap_offset += count * sector_size;
        }

        // printf("%s: start copy. src: 0x%llX dst:0x%llX, len: %d \n", __func__, src, dst, count * sector_size); //
        memcpy(dst, src, count * sector_size);

        // printf("%s: finish copy!\n\n", __func__); 

        if ((bitmap_check_trigger == true) && rw && dst)
        {
            win_CheckBitmap(dst, count * sector_size);
            printf("%s: bitmap checking...\n", __func__);
            bitmap_offset += count * sector_size;
        }

        if ((ParseMFT_trigger == true) && rw && dst)
        {
            ParseMFT(dst, count * sector_size);
        }

        if (rw && dst)
        {
            buffer_byte_count(dst, count * sector_size);
        }
        // printf("%s: count:%d  copied:%d  sector_size:%d)\n", __func__, count, count * sector_size, sector_size); 
    }

    return 0;
}

void win_DetermineTag(struct cbw *cbw, win_process_info *CurrentProcessInfo)
{

    unsigned int lba, length;
    lba = (cbw->CBWCB[2] << 24) | (cbw->CBWCB[3] << 16) | (cbw->CBWCB[4] << 8) | (cbw->CBWCB[5] << 0);
    length = cbw->dCBWDataTransferLength;

    /*
    if (CurrentProcessInfo->IrpInfo[0].FileObjectInfo.is_Bitmap || (lba > NTFS_BITMAP_START_LBA && lba < NTFS_BITMAP_END_LBA))
    {
        bitmap_check_trigger = true;
        return;
    }


    if (CurrentProcessInfo->IrpInfo[0].FileObjectInfo.is_MFT || (lba > NTFS_MFT_START_LBA && lba < NTFS_MFT_END_LBA))
    {
        win_AddTag(cbw, RANSOMTAG_MFT);
        ParseMFT_trigger = true;
        printf("%s:Add MFT tag\n", __func__);
        return;
    }

    int ret = 0;
    ret = win_SearchSectorMap(cbw);
    if (ret)
    {
        win_AddTag(cbw, RANSOMTAG);
        printf("%s: This page is in bitmap, add tag!!\n", __func__);
        return;
    }

    if(win_traverse_process_blacklist(CurrentProcessInfo->ProcessName))
    {
        win_AddTag(cbw, RANSOMTAG);
        printf("%s: Process [%s] on the blacklist, add tag!\n", __func__,CurrentProcessInfo->ProcessName);
        //win_print_process_info(CurrentProcessInfo);
        //win_print_irp_info(CurrentProcessInfo);

        return;
    }
    */
    if (CurrentProcessInfo->IrpInfo[0].FileObjectInfo.is_MFT || (lba > NTFS_MFT_START_LBA && lba < NTFS_MFT_END_LBA))
    {
        win_AddTag(cbw, RANSOMTAG_MFT);
        ParseMFT_trigger = true;
        printf("%s:Add MFT tag\n", __func__);
    }
    else
    {
        win_AddTag(cbw, RANSOMTAG);
    }
    return;
}

void win_analyse(void *usb_msc_cbw)
{
    // printf("\n\n\n***New CBW***\n");
    int ret = 0;
    struct cbw *cbw = NULL;
    cbw = (struct cbw *)usb_msc_cbw;
    if (!cbw)
    {
        // printf("cbw INVALID!\n");
        return;
    }

    unsigned int lba;
    lba = (cbw->CBWCB[2] << 24) | (cbw->CBWCB[3] << 16) | (cbw->CBWCB[4] << 8) | (cbw->CBWCB[5] << 0);

    win_process_info CurrentProcessInfo;
    win_get_CurrentProcessInfo(&CurrentProcessInfo);
    if (!is_win_kernel_addr(CurrentProcessInfo.EThread))
    {
        printf("%s: CurrentProcessInfo.EThread is INVALID\n", __func__);
        return;
    }

    traverse_IrpList(&CurrentProcessInfo, lba);
    /*
    ret = 0;
    if ((file_lba_start != 0 && file_lba_end != 0) && (lba > file_lba_end || lba < file_lba_start) && lba != 0)
    {
        if (!win_traverse_process_blacklist(ProcessForEntropyCalc) && !traverse_entropy_excluded_process_list(ProcessForEntropyCalc))
        {
            ret = entropy_calc();
        }

        file_lba_start = file_lba_end = 0;

        if (ret == HIGH_ENTROPY)
        {
            //int ret2 = 0;
            //ret2 = win_add_to_high_entropylist(ProcessForEntropyCalc);
           // if(ret2==1)
            //{
                win_add_to_process_blacklist(ProcessForEntropyCalc);
                win_del_from_patternlist(ProcessForEntropyCalc);
                printf("%s: [%s] write high entropy, add to blakclist!\n", __func__, ProcessForEntropyCalc);
            //}
        }
    }
    */
    switch (cbw->CBWCB[0])
    {
    case 0x28: /* READ(10) */
        // printf("%s: READ\n", __func__);
        /*
        if (lba == BITMAP_LBA)
        {
            win_load_bitmap();
        }

        if (!win_traverse_process_blacklist(CurrentProcessInfo.ProcessName) && !traverse_entropy_excluded_process_list(CurrentProcessInfo.ProcessName))
        {
            memset(ProcessForPattern, 0, 15);
            memcpy(ProcessForPattern, CurrentProcessInfo.ProcessName, 15);
            win_add_to_patternlist(CurrentProcessInfo.ProcessName, CurrentProcessInfo.IrpInfo[0].FileObjectInfo.FileName.FileNameStr, CurrentProcessInfo.IrpInfo[0].FileObjectInfo.FileName.Length, READFILE);
        }
        */

        win_Handle_Recovery(cbw);
        break;
    case 0x2a: /* WRITE(10) */
        /*
        if (CurrentProcessInfo.IrpInfo[0].used && (lba > file_lba_end || lba < file_lba_start) && CurrentProcessInfo.IrpInfo[0].ReadWriteLength > 0)
        {
            file_lba_start = lba;
            file_lba_end = lba + (CurrentProcessInfo.IrpInfo[0].ReadWriteLength / 512);
            entropy_length = CurrentProcessInfo.IrpInfo[0].ReadWriteLength;
            memset(ProcessForEntropyCalc, 0, 15);
            memcpy(ProcessForEntropyCalc, CurrentProcessInfo.ProcessName, 15);
            memset(byte_count, 0, 256 * sizeof(long long));
        }

        if (!win_traverse_process_blacklist(CurrentProcessInfo.ProcessName))
        {
            win_add_to_patternlist(CurrentProcessInfo.ProcessName, CurrentProcessInfo.IrpInfo[0].FileObjectInfo.FileName.FileNameStr, CurrentProcessInfo.IrpInfo[0].FileObjectInfo.FileName.Length, WRITEFILE);
        }
        */
        win_DetermineTag(cbw, &CurrentProcessInfo);

        break;

    default:
        break;
    }
}
/*************************  Windows End  ***************************************************************************************************/

/*Backup*/

/*
unsigned char urb_data[208] = {0};
    read_virtaddr_data(urb_virt,208, urb_data);
    printf("\n urb_data start[0x%llX]\n", urb_virt);
    for (int i = 0; i < 208; i++)
    {
        if (i > 0 && i % 16 == 0)
        {
            printf("\n");
        }
        else if (i > 0 && i % 4 == 0)
        {
            printf("  ");
        }
        printf("%.2X ", urb_data[i]);

    }
    printf("\nurb_data end[0x%llX]\n", urb_virt);


    printf("\nstruct ehci_qtd start:\n");
    for (int i = 0; i < sizeof(struct ehci_qtd); i++)
    {
        if (i > 0 && i % 16 == 0)
        {
            printf("\n");
        }
        else if (i > 0 && i % 4 == 0)
        {
            printf("  ");
        }
        printf("%.2X ", *index);
        index++;
    }
    printf("\nstruct ehci_qtd end!\n");
    for (int i = 0; i < 76; i++)
    {
        if (i == 0)
        {
            printf("              ");
        }
        if (i > 0 && (i + 4) % 16 == 0)
        {
            printf("\n");
        }
        else if (i > 0 && i % 4 == 0)
        {
            printf("  ");
        }
        printf("%.2X ", *index);
        index++;
    }
    printf("\n extension end [%p]\n", --index);


    u64 virt1 = *(u64 *)(((unsigned char *)qtdm->qtd) + sizeof(struct ehci_qtd) + 20);   //EndpointData
    u64 virt2 = *(u64 *)(((unsigned char *)qtdm->qtd) + sizeof(struct ehci_qtd) + 28);   //TRANSFER_CONTEXT
    //u64 virt3 = *(u64 *)(((unsigned char *)qtdm->qtd) + sizeof(struct ehci_qtd) + 36);
    //u64 virt4 = *(u64 *)(((unsigned char *)qtdm->qtd) + sizeof(struct ehci_qtd) + 44);
    //u64 virt5 = *(u64 *)(((unsigned char *)qtdm->qtd) + sizeof(struct ehci_qtd) + 52);

    printf("virt1: 0x%llX\n", virt1);
    unsigned char buffer1[256] = {0};
    read_virtaddr_data(virt1, 256, buffer1);
    for (int i = 0; i < 256; i++)
    {
        if (i > 0 && i % 16 == 0)
        {
            printf("\n");
        }
        else if (i > 0 && i % 4 == 0)
        {
            printf("  ");
        }
        printf("%.2X ", buffer1[i]);

    }

    printf("\nvirt2: 0x%llX\n", virt2);
    unsigned char buffer2[256] = {0};
    read_virtaddr_data(virt2, 256, buffer2);
    for (int i = 0; i < 256; i++)
    {
        if (i > 0 && i % 16 == 0)
        {
            printf("\n");
        }
        else if (i > 0 && i % 4 == 0)
        {
            printf("  ");
        }
        printf("%.2X ", buffer2[i]);

    }

    printf("\nvirt3: 0x%llX\n", virt3);
    unsigned char buffer3[256] = {0};
    read_virtaddr_data(virt3, 256, buffer3);
    for (int i = 0; i < 256; i++)
    {
        if (i > 0 && i % 16 == 0)
        {
            printf("\n");
        }
        else if (i > 0 && i % 4 == 0)
        {
            printf("  ");
        }
        printf("%.2X ", buffer3[i]);

    }

    printf("\nvirt4: 0x%llX\n", virt4);
    unsigned char buffer4[256] = {0};
    read_virtaddr_data(virt4, 256, buffer4);
    for (int i = 0; i < 256; i++)
    {
        if (i > 0 && i % 16 == 0)
        {
            printf("\n");
        }
        else if (i > 0 && i % 4 == 0)
        {
            printf("  ");
        }
        printf("%.2X ", buffer4[i]);

    }

    printf("\nvirt5: 0x%llX\n", virt5);
    unsigned char buffer5[256] = {0};
    read_virtaddr_data(virt5, 256, buffer5);
    for (int i = 0; i < 256; i++)
    {
        if (i > 0 && i % 16 == 0)
        {
            printf("\n");
        }
        else if (i > 0 && i % 4 == 0)
        {
            printf("  ");
        }
        printf("%.2X ", buffer5[i]);

    }

    */
