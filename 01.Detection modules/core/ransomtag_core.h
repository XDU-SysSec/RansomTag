#ifndef _CORE_RANSOMTAG_CORE_H
#define _CORE_RANSOMTAG_CORE_H

#include <share/vmm_types.h>
#include "thread.h"
#include "../drivers/usb/ransomtag.h"
#include "printf.h"
#include <stdint.h>


typedef unsigned char *PUINT8;
#define IS_SYSRET_INSTRUCTION(Code)   \
    (*((PUINT8)(Code) + 0) == 0x48 && \
     *((PUINT8)(Code) + 1) == 0x0F && \
     *((PUINT8)(Code) + 2) == 0x07)
#define IS_SYSCALL_INSTRUCTION(Code) \
    (*((PUINT8)(Code) + 0) == 0x0F && \
     *((PUINT8)(Code) + 1) == 0x05)


#ifdef __ASSEMBLY__
#define _AC(X,Y)	X
#define _AT(T,X)	X
#else
#define __AC(X,Y)	(X##Y)
#define _AC(X,Y)	__AC(X,Y)
#define _AT(T,X)	((T)(X))
#endif

#define _UL(x)		(_AC(x, UL))
#define _ULL(x)		(_AC(x, ULL))

#define _BITUL(x)	(_UL(1) << (x))
#define _BITULL(x)	(_ULL(1) << (x))

#define UL(x)		(_UL(x))
#define ULL(x)		(_ULL(x))

//#define BIT(nr)			(UL(1) << (nr))
#define BIT_ULL(nr)		(ULL(1) << (nr))
#define BIT_MASK(nr)		(UL(1) << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr)		((nr) / BITS_PER_LONG)
#define BIT_ULL_MASK(nr)	(ULL(1) << ((nr) % BITS_PER_LONG_LONG))
#define BIT_ULL_WORD(nr)	((nr) / BITS_PER_LONG_LONG)
#define BITS_PER_BYTE		8

#define PAGE_SIZE       4096
#define PAGE_SHIFT		12


#define __PAGE_OFFSET           _AC(0xffff880000000000, UL)
#define PAGE_OFFSET             ((unsigned long long)__PAGE_OFFSET)
#define linux_va(x)             ((unsigned long long)(x)+PAGE_OFFSET)



/*
//intel PT
#define MSR_IA32_RTIT_CTL		0x00000570
#define RTIT_CTL_TRACEEN		BIT(0)
#define RTIT_CTL_CYCLEACC		BIT(1)
#define RTIT_CTL_OS			    BIT(2)
#define RTIT_CTL_USR			BIT(3)
#define RTIT_CTL_PWR_EVT_EN		BIT(4)
#define RTIT_CTL_FUP_ON_PTW		BIT(5)
#define RTIT_CTL_FABRIC_EN		BIT(6)
#define RTIT_CTL_CR3EN			BIT(7)
#define RTIT_CTL_TOPA			BIT(8)
#define RTIT_CTL_MTC_EN			BIT(9)
#define RTIT_CTL_TSC_EN			BIT(10)
#define RTIT_CTL_DISRETC		BIT(11)
#define RTIT_CTL_PTW_EN			BIT(12)
#define RTIT_CTL_BRANCH_EN		BIT(13)
#define RTIT_CTL_MTC_RANGE_OFFSET	14
#define RTIT_CTL_MTC_RANGE		(0x0full << RTIT_CTL_MTC_RANGE_OFFSET)
#define RTIT_CTL_CYC_THRESH_OFFSET	19
#define RTIT_CTL_CYC_THRESH		(0x0full << RTIT_CTL_CYC_THRESH_OFFSET)
#define RTIT_CTL_PSB_FREQ_OFFSET	24
#define RTIT_CTL_PSB_FREQ		(0x0full << RTIT_CTL_PSB_FREQ_OFFSET)
#define RTIT_CTL_ADDR0_OFFSET		32
#define RTIT_CTL_ADDR0			(0x0full << RTIT_CTL_ADDR0_OFFSET)
#define RTIT_CTL_ADDR1_OFFSET		36
#define RTIT_CTL_ADDR1			(0x0full << RTIT_CTL_ADDR1_OFFSET)
#define RTIT_CTL_ADDR2_OFFSET		40
#define RTIT_CTL_ADDR2			(0x0full << RTIT_CTL_ADDR2_OFFSET)
#define RTIT_CTL_ADDR3_OFFSET		44
#define RTIT_CTL_ADDR3			(0x0full << RTIT_CTL_ADDR3_OFFSET)
#define MSR_IA32_RTIT_STATUS		0x00000571
#define RTIT_STATUS_FILTEREN		BIT(0)
#define RTIT_STATUS_CONTEXTEN		BIT(1)
#define RTIT_STATUS_TRIGGEREN		BIT(2)
#define RTIT_STATUS_BUFFOVF		BIT(3)
#define RTIT_STATUS_ERROR		BIT(4)
#define RTIT_STATUS_STOPPED		BIT(5)
#define RTIT_STATUS_BYTECNT_OFFSET	32
#define RTIT_STATUS_BYTECNT		(0x1ffffull << RTIT_STATUS_BYTECNT_OFFSET)
#define ADDR0_SHIFT	32
#define ADDR1_SHIFT	36
#define ADDR2_SHIFT	40
#define ADDR3_SHIFT	44
#define ADDR0_MASK	(0xfULL << ADDR0_SHIFT)
#define ADDR1_MASK	(0xfULL << ADDR1_SHIFT)
#define ADDR2_MASK	(0xfULL << ADDR2_SHIFT)
#define ADDR3_MASK	(0xfULL << ADDR3_SHIFT)
#define MSR_IA32_RTIT_ADDR0_A		0x00000580
#define MSR_IA32_RTIT_ADDR0_B		0x00000581
#define MSR_IA32_RTIT_ADDR1_A		0x00000582
#define MSR_IA32_RTIT_ADDR1_B		0x00000583
#define MSR_IA32_RTIT_ADDR2_A		0x00000584
#define MSR_IA32_RTIT_ADDR2_B		0x00000585
#define MSR_IA32_RTIT_ADDR3_A		0x00000586
#define MSR_IA32_RTIT_ADDR3_B		0x00000587
#define MSR_IA32_RTIT_CR3_MATCH		0x00000572
//#define MSR_IA32_RTIT_OUTPUT_BASE	0x00000560
//#define MSR_IA32_RTIT_OUTPUT_MASK_PTRS	0x00000561
#define TOPA_STOP	BIT_ULL(4)
#define TOPA_INT	BIT_ULL(2)
#define TOPA_END	BIT_ULL(0)
#define TOPA_SIZE_SHIFT 6
*/

/* Syscall number (linux 5.4) */
// read-related
#define __NR_read 63
#define __NR_readv 65
#define __NR_preadv 69
// write-related
#define __NR_write 64
#define __NR_pwrite64 68
#define __NR_pwritev 70

struct syscall_sysret_opcode {
	u8 code1;
	u8 code2;
	u8 code3;
};

extern int pt_buffer_order;
extern void *pt_buffer_array[16];
extern u64 *topa_array[16];
extern tid_t pt_decoder_thread_tid[16];

unsigned long long read_pt_status(void);
void print_pt_status(void);
unsigned long long read_pt_ctl(void);
void write_pt_ctl(u64 ctl);
int pt_buffer_init(int cpu);
void init_pt_buffer_msrs(int cpu);
void stop_pt(int cpu);
void start_pt(void);
void print_pt_buffer(void);

void pt_decoder_thread(void *arg);
int packet_decode(void);
void pt_init(void);

void get_current_rsp_reg(unsigned long *rsp, unsigned long *rbp); 
void get_current_gs_reg(unsigned long long *gs);
void get_current_cr3_reg(unsigned long long *cr3);
void get_current_fs_reg(unsigned long long *fs);
void get_msr_gs_base(unsigned long long *gs_base);
int get_init_task_stack(unsigned long *init_task_stack);
void get_msr_efer(unsigned long long *efer);
int SetMsrBitmap(u64 Msr, int ReadDetection, int WriteDetection);
void ransomtag_read_opcode(struct syscall_sysret_opcode *opcode);

int handle_UD(void);

#endif
