#include "ransomtag_core.h"
#include "current.h"
#include "../drivers/usb/ransomtag.h"
#include "pcpu.h"
#include "mm.h"
#include "string.h"
#include "initfunc.h"


unsigned char linux_excluded_processes[][15] = {
    "swapper",
    "modprobe",
   // "kworker",
   // "systemd",
   // "scsi_eh",
   // "fsck",
   // "ext4lazyinit",
   // "pool-udisksd",
   // "jbd2",
   // "usb-storage",
   // "pool-tracker",
    
    };


static inline unsigned char *core_mem_find_str(unsigned char *full_data, int full_data_len, unsigned char *substr)
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


//others
int get_init_task_stack(unsigned long *init_task_stack)
{
    if ((current->vcpu0->u.vt.vr.rbp) > 0xFFFFFFFF00000000)
    {
        *init_task_stack = current->vcpu0->u.vt.vr.rbp & ~(0x4000 - 1);
        return 1;
    }
    else
    {
        return -1; //rbp is invalid
    }
}

void get_current_rsp_reg(unsigned long *rsp, unsigned long *rbp)
{

    current->vmctl.read_general_reg(GENERAL_REG_RSP, rsp);
    current->vmctl.read_general_reg(GENERAL_REG_RBP, rbp);
}

void get_current_gs_reg(unsigned long long *gs)
{
    current->vmctl.read_sreg_base(SREG_GS, gs);
}

void get_current_cr3_reg(unsigned long long *cr3)
{
    current->vmctl.read_control_reg(CONTROL_REG_CR3, cr3);
}

void get_current_fs_reg(unsigned long long *fs)
{
    current->vmctl.read_sreg_base(SREG_FS, fs);
}


void get_msr_gs_base(unsigned long long *gs_base)
{
    current->vmctl.read_msr(MSR_IA32_GS_BASE, gs_base);
}


void find_task_from_cr3(unsigned long long cr3)
{
    
}

void get_msr_efer(unsigned long long *efer)
{
    current->vmctl.read_msr(MSR_IA32_EFER, efer);
}

static void SetBit(void *Addr, u64 Bit, bool Set)
{
    u64 Byte = Bit / 8;
    u64 Temp = Bit % 8;
    u64 N = 7 - Temp;

    unsigned char *Addr2 = Addr;

    if (Set)
    {
        Addr2[Byte] |= (1 << N);
    }
    else
    {
        Addr2[Byte] &= ~(1 << N);
    }
}

static void GetBit(void *Addr, u64 Bit)
{
    u64 Byte = 0, K = 0;
    Byte = Bit / 8;
    K = 7 - Bit % 8;
    unsigned char *Addr2 = Addr;

    Addr2[Byte] & (1 << K);
}

int SetMsrBitmap(u64 Msr, int ReadDetection, int WriteDetection)
{
    if (!ReadDetection && !WriteDetection)
    {
        //
        // Invalid Command
        //
        return 0;
    }

    if (Msr <= 0x00001FFF)
    {
        if (ReadDetection == 1)
        {
            SetBit(current->u.vt.msrbmp->msrbmp, Msr, true);
        }
        if (WriteDetection == 1)
        {
            SetBit((unsigned char *)current->u.vt.msrbmp->msrbmp + 2048, Msr, true);
        }
    }
    else if ((0xC0000000 <= Msr) && (Msr <= 0xC0001FFF))
    {
        if (ReadDetection == 1)
        {
            SetBit((unsigned char *)current->u.vt.msrbmp->msrbmp + 1024, Msr - 0xC0000000, true);
        }
        if (WriteDetection == 1)
        {
            SetBit((unsigned char *)current->u.vt.msrbmp->msrbmp + 3072, Msr - 0xC0000000, true);
        }
    }
    else
    {
        return 0;
    }
    return 0;
}

void HandleMSRRead(void)
{
    u64 msr = 0;
    ulong rcx;

    current->vmctl.read_general_reg (GENERAL_REG_RCX, &rcx);

    //
    // RDMSR. The RDMSR instruction causes a VM exit if any of the following are true:
    //
    // The "use MSR bitmaps" VM-execution control is 0.
    // The value of ECX is not in the ranges 00000000H - 00001FFFH and C0000000H - C0001FFFH
    // The value of ECX is in the range 00000000H - 00001FFFH and bit n in read bitmap for low MSRs is 1,
    //   where n is the value of ECX.
    // The value of ECX is in the range C0000000H - C0001FFFH and bit n in read bitmap for high MSRs is 1,
    //   where n is the value of ECX & 00001FFFH.
    //

    if (((rcx <= 0x00001FFF)) || ((0xC0000000 <= rcx) && (rcx <= 0xC0001FFF)))
    {
        vt_read_msr(rcx, &msr);
    }

    current->vmctl.write_general_reg (GENERAL_REG_RAX, *(ulong *)(&msr));   //msr.low
    current->vmctl.write_general_reg(GENERAL_REG_RDX, *((ulong *)(&msr) + 1));   //msr.high
}

void HandleMSRWrite(void)
{
    u64 msr = 0;
    ulong rax = 0, rdx = 0, rcx = 0;
    ulong *p = (ulong *)&msr;

    current->vmctl.read_general_reg (GENERAL_REG_RAX, &rax);
    current->vmctl.read_general_reg (GENERAL_REG_RDX, &rdx);

    //
    // Check for the sanity of MSR
    //
    if ((rcx <= 0x00001FFF) || ((0xC0000000 <= rcx) && (rcx <= 0xC0001FFF)))
    {
        *p = rax;
        *(p + 1) = rdx;
        current->vmctl.write_general_reg(GENERAL_REG_RCX, msr);
    }
}

static inline int core_is_linux_kernel_addr(linux_virtaddr addr)
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

static inline int core_read_virtaddr_data(linux_virtaddr virt_addr, int read_len, void *data)
{
    
    if(!core_is_linux_kernel_addr(virt_addr) || !data)
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

static inline int core_get_linux_task_struct_info(linux_virtaddr task_struct,linux_task_info_ptr linux_task_info)  
{
    int ret = 0;
    linux_task_info->task_struct = task_struct;
    
    core_read_virtaddr_data(task_struct + TASK_STRUCT_STACK_OFFSET, LINUX_ADDR_SIZE, &linux_task_info->stack);
    core_read_virtaddr_data(task_struct + TASK_STRUCT_FILES_OFFSET, LINUX_ADDR_SIZE, &linux_task_info->files);
    core_read_virtaddr_data(linux_task_info->files + FILES_STRUCT_FDT_OFFSET, LINUX_ADDR_SIZE, &linux_task_info->fdtable);
    core_read_virtaddr_data(task_struct + TASK_STRUCT_TASKS_OFFSET, LINUX_ADDR_SIZE, &linux_task_info->list_head_tasks.next);
    core_read_virtaddr_data(task_struct + TASK_STRUCT_TASKS_OFFSET + LINUX_ADDR_SIZE, LINUX_ADDR_SIZE, &linux_task_info->list_head_tasks.prev);
    core_read_virtaddr_data(task_struct + TASK_STRUCT_PID_OFFSET, LINUX_ADDR_SIZE, &linux_task_info->pid);
    core_read_virtaddr_data(task_struct + TASK_STRUCT_PARENT_OFFSET, LINUX_ADDR_SIZE, &linux_task_info->parent_task_struct);
    core_read_virtaddr_data(linux_task_info->parent_task_struct + TASK_STRUCT_PID_OFFSET, LINUX_ADDR_SIZE, &linux_task_info->parent_pid);
    core_read_virtaddr_data(task_struct + TASK_STRUCT_COMM_OFFSET,16,linux_task_info->task_name);
    core_read_virtaddr_data(task_struct + TASK_STRUCT_MM_OFFSET, LINUX_ADDR_SIZE, &linux_task_info->mm);
    core_read_virtaddr_data(linux_task_info->mm + MM_STRUCT_PGD_OFFSET, LINUX_ADDR_SIZE, &linux_task_info->mm_pgd);
    core_read_virtaddr_data(task_struct + TASK_STRUCT_ACTIVE_MM_OFFSET, LINUX_ADDR_SIZE, &linux_task_info->active_mm);
    core_read_virtaddr_data(linux_task_info->active_mm + MM_STRUCT_PGD_OFFSET, LINUX_ADDR_SIZE, &linux_task_info->active_mm_pgd);

    linux_task_info->mm_pgd_phys = guestvirt_to_guestphys(linux_task_info->mm_pgd);
    linux_task_info->active_mm_pgd_phys = guestvirt_to_guestphys(linux_task_info->active_mm_pgd);

    linux_task_info->list_head_tasks.list_head_field = task_struct + TASK_STRUCT_TASKS_OFFSET;

    return ret;
}


int core_linux_traverse_excluded_process_list(unsigned char *ProcessName)
{
    int i, count;
    count = sizeof(linux_excluded_processes) / sizeof(linux_excluded_processes[0]);
    for (i = 0; i < count; i++)
    {
        if (core_mem_find_str(ProcessName, 31, linux_excluded_processes[i]))
        {
            return 1;
        }
    }
    return 0;
}

void core_print_task_info(linux_task_info_ptr linux_task_info)
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


//
// SYSCALL instruction emulation routine
//
static inline bool EmulateSYSCALL(void)
{
    //printf("%s: \n", __func__);
    uint64_t msr_lstar;
    uint64_t rip;
    ulong instruction_len;
    //
    // Save the address of the instruction following SYSCALL into RCX and then
    // load RIP from MSR_LSTAR.
    //
    current->vmctl.read_msr(MSR_IA32_LSTAR, &msr_lstar);
    current->vmctl.read_ip(&rip);
    asm_vmread (VMCS_VMEXIT_INSTRUCTION_LEN, &instruction_len);
    rip += instruction_len;
    current->vmctl.write_general_reg(GENERAL_REG_RCX, rip);
    current->vmctl.write_ip(msr_lstar);


    //
    // Save RFLAGS into R11 and then mask RFLAGS using MSR_FMASK.
    //
    uint64_t fmask;
    uint64_t rflags;
    current->vmctl.read_flags(&rflags);
    current->vmctl.write_general_reg(GENERAL_REG_R11, rflags);
    current->vmctl.read_msr(MSR_IA32_FMASK, &fmask);
    rflags &= ~(fmask | RFLAGS_RF_BIT);
    current->vmctl.write_flags(rflags);

    //
    // Load the CS and SS selectors with values derived from bits 47:32 of MSR_STAR.
    //
    uint64_t msr_star;
    current->vmctl.read_msr(MSR_IA32_STAR, &msr_star);
    
    asm_vmwrite (VMCS_GUEST_CS_SEL, (uint16_t)((msr_star >> 32) & ~3));    //// STAR[47:32] & ~RPL3
    asm_vmwrite (VMCS_GUEST_CS_BASE, 0);                        // flat segment
    asm_vmwrite (VMCS_GUEST_CS_LIMIT, 0xFFFFFFFF);            // 4GB limit
    asm_vmwrite (VMCS_GUEST_CS_ACCESS_RIGHTS, 0xA09B);              // L+DB+P+S+DPL0+Code
    
    asm_vmwrite (VMCS_GUEST_SS_SEL, (uint16_t)(((msr_star >> 32) & ~3) + 8));       // STAR[47:32] + 8
    asm_vmwrite (VMCS_GUEST_SS_BASE, 0);                    // flat segment
    asm_vmwrite (VMCS_GUEST_SS_LIMIT, 0xFFFFFFFF);            // 4GB limit
    asm_vmwrite (VMCS_GUEST_SS_ACCESS_RIGHTS, 0xC093);           // G+DB+P+S+DPL0+Data
    
    //printf("%s: handle SYSCALL done!\n", __func__);
    return true;
}



//
// SYSRET instruction emulation routine
//
static inline bool EmulateSYSRET(void)
{
    //printf("%s: \n", __func__);
    uint64_t msr_lstar;
    uint64_t rip;

    //
    // Load RIP from RCX.
    //
    current->vmctl.read_general_reg(GENERAL_REG_RCX, &rip);
    current->vmctl.write_ip(rip);
    


    //
    // Load RFLAGS from R11. Clear RF, VM, reserved bits.
    //
    uint64_t rflags;
    uint64_t r11;

    current->vmctl.read_general_reg(GENERAL_REG_R11, &r11);
    rflags = (r11 & ~(RFLAGS_RF_BIT | RFLAGS_VM_BIT | RFLAGS_RESERVED_BIT)) | RFLAGS_ALWAYS1_BIT;
    current->vmctl.write_flags(rflags);



    //
    // SYSRET loads the CS and SS selectors with values derived from bits 63:48 of MSR_STAR.
    //
    uint64_t msr_star;
    current->vmctl.read_msr(MSR_IA32_STAR, &msr_star);

    asm_vmwrite (VMCS_GUEST_CS_SEL, (uint16_t)(((msr_star >> 48) + 16) | 3));    // (STAR[63:48]+16) | 3 (* RPL forced to 3 *)
    asm_vmwrite (VMCS_GUEST_CS_BASE, 0);                    // Flat segment
    asm_vmwrite (VMCS_GUEST_CS_LIMIT, 0xFFFFFFFF);            // 4GB limit
    asm_vmwrite (VMCS_GUEST_CS_ACCESS_RIGHTS, 0xA0FB);           // L+DB+P+S+DPL3+Code
    
    asm_vmwrite (VMCS_GUEST_SS_SEL, (uint16_t)(((msr_star >> 48) + 8) | 3));     // (STAR[63:48]+8) | 3 (* RPL forced to 3 *)
    asm_vmwrite (VMCS_GUEST_SS_BASE, 0);                // Flat segment
    asm_vmwrite (VMCS_GUEST_SS_LIMIT, 0xFFFFFFFF);            // 4GB limit
    asm_vmwrite (VMCS_GUEST_SS_ACCESS_RIGHTS, 0xC0F3);           // G+DB+P+S+DPL3+Data

    //printf("%s: handle SYSRET done!\n", __func__);

    return true;
}

static inline bool is_syscall_instruction(void)
{
    struct syscall_sysret_opcode opcode = {0};
    ransomtag_read_opcode(&opcode);
    
    
    if (opcode.code1 == 0x0F && opcode.code2 == 0x05)
    {
        //printf("%s: SYSCALL code1: 0x%X, code2: 0x%X\n", __func__, opcode.code1, opcode.code2);
        return true;
    }

    return false;
}

static inline bool is_sysret_instruction(void)
{
    //printf("%s: \n", __func__);
    struct syscall_sysret_opcode opcode = {0};
    ransomtag_read_opcode(&opcode);
    if (opcode.code1 == 0x48 && opcode.code2 == 0x0F && opcode.code3 == 0x07){
        //printf("%s: SYSRET code1: 0x%X, code2: 0x%X, code3: 0x%X\n", __func__, opcode.code1, opcode.code2, opcode.code3);
    
        return true;
    }
    //printf("%s: return false\n", __func__);
    
    return false;
        
}

static inline linux_virtaddr core_get_current_task(void)
{
    u64 gs_base;

    // before SWAPGS, the address is in MSR_IA32_KERNEL_GS_BASE instead of MSR_IA32_GS_BASE
    current->vmctl.read_msr(MSR_IA32_KERNEL_GS_BASE, &gs_base);

    ulong current_task = gs_base + CURRENT_TASK_GS_OFFSET;
    linux_virtaddr task_struct_addr;
    int ret = 0;

    ret = core_read_virtaddr_data(current_task, LINUX_ADDR_SIZE, &task_struct_addr);
    //printf("%s: gs_base:0x%llX, current_task: 0x%llX\n", __func__, gs_base, current_task);
    if (ret && core_is_linux_kernel_addr(task_struct_addr))
    {
        linux_task_info task_info;
        core_get_linux_task_struct_info(task_struct_addr, &task_info);
        /*
        if (!core_linux_traverse_excluded_process_list(task_info.task_name))
        {
            core_print_task_info(&task_info);
        }
        */
    }

    return task_struct_addr;
}

static inline void get_ctx_information(void)
{
    // regs->di,regs->si,regs->dx,regs->r10,regs->r8,regs->r9
    
    ulong rax;

    current->vmctl.read_general_reg(GENERAL_REG_RAX, &rax);   //get syscall number

    switch (rax)
    {
    // write-related
    case __NR_write:
    case __NR_pwrite64:
    case __NR_pwritev:
        core_get_current_task();
        //printf("%s: syscall number: %d\n", __func__, rax);
        break;

    default:
        break;
    }



    /*
    if(rax==1)
    {
        core_get_current_task();
    }
    */   

}

int handle_UD(void)
{
    if (is_syscall_instruction())
    {
        // get the OS context information
        //get_ctx_information();
        
        return EmulateSYSCALL(); // Emulate SYSCALL instruction.
    }
    //printf("%s: not SYSCALL\n", __func__);
    if (is_sysret_instruction())
    {
        return EmulateSYSRET(); // Emulate SYSRET instruction.
    }

    return 0;
}
