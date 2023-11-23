/***********************************************************************
 * $Id:: lpc_arm922t_arch.h 745 2008-05-13 19:59:29Z pdurgesh          $
 *
 * Project: General Utilities
 *
 * Description:
 *      This file contains constant and macro definitions specific
 *      to the ARM922T architecture.
 *
 ***********************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
 **********************************************************************/
 
#ifndef LPC_ARM922T_ARCH_H
#define LPC_ARM922T_ARCH_H

#ifdef __GNUC__
/* GNU does not support these macros and will use the direct
   coprocessor IDs when using the MRC and MCR instructions */

#else /* ARM and GHS tools use these defines */

/* System Control Coprocessor definitions */
/* These symbols are used as arguments for the <coproc> parameter
   in MCR and MRC instructions */

/* ARM and GHS tool coprocessor define: system control */
#define ARM922T_SYS_CONTROL_CP      p15
/* ARM and GHS tool coprocessor define: MMU */
#define ARM922T_MMU_CP              p15
/* ARM and GHS tool coprocessor define: cache */
#define ARM922T_CACHE_CP            p15

/* Valid CP15 Coprocessor Registers */
/* These symbols are used as arguments for the <CRn> parameter
 * in MCR and MRC instructions */

/* ARM and GHS tool coprocessor define: ID code register */
#define ARM922T_MMU_REG_ID          c0
/* ARM and GHS tool coprocessor define: cache type register */
#define ARM922T_MMU_REG_CACHE_TYPE  c0
/* ARM and GHS tool coprocessor define: control register */
#define ARM922T_MMU_REG_CONTROL     c1
/* ARM and GHS tool coprocessor define: translation table base reg */
#define ARM922T_MMU_REG_TTB         c2
/* ARM and GHS tool coprocessor define: domain control register */
#define ARM922T_MMU_REG_DAC         c3
/* ARM and GHS tool coprocessor define: fault status registers */
#define ARM922T_MMU_REG_FAULT_STATUS c5
/* ARM and GHS tool coprocessor define: fault address register */
#define ARM922T_MMU_REG_FAULT_ADDRESS c6
/* ARM and GHS tool coprocessor define: cache operations register */
#define ARM922T_MMU_REG_CACHE_OPS   c7
/* ARM and GHS tool coprocessor define: TLB operations register */
#define ARM922T_MMU_REG_TLB_OPS     c8
/* ARM and GHS tool coprocessor define: cache lockdown register */
#define ARM922T_MMU_REG_CACHE_LOCKDOWN c9
/* ARM and GHS tool coprocessor define: TLB operations register */
#define ARM922T_MMU_REG_TLB_LOCKDOWN c10
/* ARM and GHS tool coprocessor define: FCSE PID register */
#define ARM922T_MMU_REG_FSCE_PID    c13
#endif

/***********************************************************************
 * MMU Control register fields
***********************************************************************/

/* ARM922T MMU control register MMU enable bit */
#define ARM922T_MMU_CONTROL_M       0x00000001
/* ARM922T MMU control register alignment fault bit */
#define ARM922T_MMU_CONTROL_A       0x00000002
/* ARM922T MMU control register data cache bit */
#define ARM922T_MMU_CONTROL_C       0x00000004
/* ARM922T MMU control register system protection bit */
#define ARM922T_MMU_CONTROL_S       0x00000100
/* ARM922T MMU control register ROM protection bit */
#define ARM922T_MMU_CONTROL_R       0x00000200
/* ARM922T MMU control register instruction cache bit */
#define ARM922T_MMU_CONTROL_I       0x00001000
/* ARM922T MMU control register vector relocation bit */
#define ARM922T_MMU_CONTROL_V       0x00002000
/* ARM922T MMU control register round robin replacement bit */
#define ARM922T_MMU_CONTROL_RR      0x00004000
/* ARM922T MMU control register 'Not FastBus' bit */
#define ARM922T_MMU_CONTROL_NF      0x40000000
/* ARM922T MMU control register 'Asynchronous Clock Select' bit */
#define ARM922T_MMU_CONTROL_IA      0x80000000

/* ARM922T MMU control register 'FastBus' mode */
#define ARM922T_MMU_CONTROL_FASTBUS 0x00000000
/* ARM922T MMU control register 'Synchronous bus' mode */
#define ARM922T_MMU_CONTROL_SYNC    0x40000000
/* ARM922T MMU control register 'Asynchronous bus' mode */
#define ARM922T_MMU_CONTROL_ASYNC   0xC0000000
/* ARM922T MMU control register bus mode mask */
#define ARM922T_MMU_CONTROL_BUSMASK 0x3FFFFFFF

/***********************************************************************
 * ARM920T/ARM922T MMU Cache type register fields
***********************************************************************/
/*  DCache Size */
#define ARM922T_MMU_DC_SIZE(n)      (((n) >> 18) & 0x7)
/*  ICache Size */
#define ARM922T_MMU_IC_SIZE(n)      (((n) >> 6) & 0x7)

/***********************************************************************
 * MMU Domain access control register fields
***********************************************************************/
/* ARM922T MMU domain register 'no access' ID field */
#define ARM922T_MMU_DN_NONE         0
/* ARM922T MMU domain register 'client access' ID field */
#define ARM922T_MMU_DN_CLIENT       1
/* ARM922T MMU domain register 'all access' ID field */
#define ARM922T_MMU_DN_MANAGER      3
/* ARM922T MMU domain register load macro for domain and access */
#define ARM922T_MMU_DN_ACCESS(n,m)  ((m & 0x3) << ((n) * 2))

/***********************************************************************
 * MMU fault status register fields
***********************************************************************/

/* ARM922T MMU fault status register fault domain load macro */
#define ARM922T_MMU_FSR_DOMAIN(n)   (((n) & 0xF0) >> 4)
/* ARM922T MMU fault status register fault type load macro */
#define ARM922T_MMU_FSR_TYPE(n)     ((n) & 0x0F)

/***********************************************************************
 * MMU Translation table register fields
***********************************************************************/

/* Number of entries in ARM922T Translation table */
#define ARM922T_TT_ENTRIES          4096
/* Size of the ARM922T Translation table */
#define ARM922T_TT_SIZE             (ARM922T_TT_ENTRIES * 4)
/* Number of entries in ARM922T coarse page table */
#define ARM922T_CPT_ENTRIES         256
/* Size of the ARM922T coarse page table */
#define ARM922T_CPT_SIZE            (ARM922T_CPT_ENTRIES * 4)
/* Mask to get the coarse page table index */
#define ARM922T_CPT_INDEX_MASK      (ARM922T_CPT_ENTRIES - 1)
/* Number of entries in ARM922T fine page table */
#define ARM922T_FPT_ENTRIES         1024
/* Size of the ARM922T fine page table */
#define ARM922T_FPT_SIZE            (ARM922T_FPT_ENTRIES * 4)
/* Mask to get the fine page table index */
#define ARM922T_FPT_INDEX_MASK      (ARM922T_FPT_ENTRIES - 1)
/* Level 1 translation table address mask */
#define ARM922T_TT_ADDR_MASK        0xFFFFC000

/***********************************************************************
 * MMU translating page table fields
***********************************************************************/

/* Level 1 Descriptor fields */
/* ARM922T MMU level 1 invalid page or section identifier */
#define ARM922T_L1D_TYPE_FAULT      0x00000000
/* ARM922T MMU level 1 coarse page identifier */
#define ARM922T_L1D_TYPE_CPAGE      0x00000001
/* ARM922T MMU level 1 fine page identifier */
#define ARM922T_L1D_TYPE_FPAGE      0x00000003
/* ARM922T MMU level 1 1MByte section identifier */
#define ARM922T_L1D_TYPE_SECTION    0x00000002
/* ARM922T MMU level 1 compatibility bit */
#define ARM922T_L1D_TYPE_PG_SN_MASK 0x00000003
/* ARM922T MMU level 1 page or section bufferable bit */
#define ARM922T_L1D_COMP_BIT        0x00000010
/* ARM922T MMU level 1 page or section identifier */
#define ARM922T_L1D_BUFFERABLE      0x00000004
/* ARM922T MMU level 1 page or section cacheable bit */
#define ARM922T_L1D_CACHEABLE       0x00000008
/* ARM922T MMU level 1 page or section domain load macro */
#define ARM922T_L1D_DOMAIN(n)       _SBF(5, ((n) & 0x0F))
/* ARM922T MMU level 1 section service access only bit */
#define ARM922T_L1D_AP_SVC_ONLY     0x00000400
/* ARM922T MMU level 1 section client read-only access only bit */
#define ARM922T_L1D_AP_USR_RO       0x00000800
/* ARM922T MMU level 1 section all access bit */
#define ARM922T_L1D_AP_ALL          0x00000C00
/* ARM922T MMU level 1 section address load macro */
#define ARM922T_L1D_SN_BASE_ADDR(n) _SBF(20, ((n) & 0x00000FFF))
/* ARM922T MMU level 2 large page address mask */
#define ARM922T_L2D_SN_BASE_MASK    0xFFF00000
/* ARM922T MMU level 1 coarse page address load macro */
#define ARM922T_L1D_CP_BASE_ADDR(n) _SBF(10, ((n) & 0x003FFFFF))
/* ARM922T MMU level 2 large page address mask */
#define ARM922T_L2D_CP_BASE_MASK    0xFFFFFC00
/* ARM922T MMU level 1 fine page address load macro */
#define ARM922T_L1D_FP_BASE_ADDR(n) _SBF(12, ((n) & 0x000FFFFF))
/* ARM922T MMU level 2 fine page address mask */
#define ARM922T_L2D_FP_BASE_MASK    0xFFFFF000

/* Level 2 Descriptor fields */
/* ARM922T MMU level 2 invalid page (fault) identifier */
#define ARM922T_L2D_TYPE_FAULT      0x00000000
/* ARM922T MMU level 2 large page (fault) identifier */
#define ARM922T_L2D_TYPE_LARGE_PAGE 0x00000001
/* ARM922T MMU level 2 small page (fault) identifier */
#define ARM922T_L2D_TYPE_SMALL_PAGE 0x00000002
/* ARM922T MMU level 2 tiny page (fault) identifier */
#define ARM922T_L2D_TYPE_TINY_PAGE  0x00000003
/* ARM922T MMU level 2 page mask */
#define ARM922T_L2D_TYPE_PAGE_MASK  0x00000003
/* ARM922T MMU level 2 page buffer enable bit */
#define ARM922T_L2D_BUFFERABLE      0x00000004
/* ARM922T MMU level 2 page cache enable bit */
#define ARM922T_L2D_CACHEABLE       0x00000008
/* ARM922T MMU level 2 section AP0 service access only bit */
#define ARM922T_L2D_AP0_SVC_ONLY    0x00000010
/* ARM922T MMU level 2 section AP0 client read-only access only bit */
#define ARM922T_L2D_AP0_USR_RO      0x00000020
/* ARM922T MMU level 2 section AP0 all access bit */
#define ARM922T_L2D_AP0_ALL         0x00000030
/* ARM922T MMU level 2 section AP1 service access only bit */
#define ARM922T_L2D_AP1_SVC_ONLY    0x00000040
/* ARM922T MMU level 2 section AP1 client read-only access only bit */
#define ARM922T_L2D_AP1_USR_RO      0x00000080
/* ARM922T MMU level 2 section AP1 all access bit */
#define ARM922T_L2D_AP1_ALL         0x000000C0
/* ARM922T MMU level 2 section AP2 service access only bit */
#define ARM922T_L2D_AP2_SVC_ONLY    _SBF(8,1)
/* ARM922T MMU level 2 section AP2 client read-only access only bit */
#define ARM922T_L2D_AP2_USR_RO      _SBF(8,2)
/* ARM922T MMU level 2 section AP2 all access bit */
#define ARM922T_L2D_AP2_ALL         _SBF(8,3)
/* ARM922T MMU level 3 section AP2 service access only bit */
#define ARM922T_L2D_AP3_SVC_ONLY    _SBF(10,1)
/* ARM922T MMU level 3 section AP2 client read-only access only bit */
#define ARM922T_L2D_AP3_USR_RO      _SBF(10,2)
/* ARM922T MMU level 3 section AP2 all access bit */
#define ARM922T_L2D_AP3_ALL         _SBF(10,3)
/* ARM922T MMU level 2 large page address load macro */
#define ARM922T_L2D_LPAGE_ADDR(n)   _SBF(16, ((n) & 0x0000FFFF)
/* ARM922T MMU level 2 large page address mask */
#define ARM922T_L2D_LPAGE_MASK      0xFFFF0000
/* ARM922T MMU level 2 small page address load macro */
#define ARM922T_L2D_SPAGE_ADDR(n)   _SBF(12, ((n) & 0x000FFFFF)
/* ARM922T MMU level 2 small page address mask */
#define ARM922T_L2D_SPAGE_MASK      0xFFFFF000
/* ARM922T MMU level 2 tiny page address load macro */
#define ARM922T_L2D_TPAGE_ADDR(n)   _SBF(10, ((n) & 0x003FFFFF)
/* ARM922T MMU level 2 tiny page address mask */
#define ARM922T_L2D_TPAGE_MASK      0xFFFFFC00

#endif /* LPC_ARM922T_ARCH_H */
