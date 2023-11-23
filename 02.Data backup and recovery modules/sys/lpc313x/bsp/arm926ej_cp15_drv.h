/*************************************************************************
 *
 *    Used with ICCARM and AARM.
 *
 *    (c) Copyright IAR Systems 2006
 *
 *    File name   : arm926ej_cp15_drv.h
 *    Description : Definitions of a driver for the ARM926EJ's CP15
 *
 *    History :
 *    1. Date        : September, 8 2006
 *       Author      : Stanimir Bonev
 *       Description : Create
 *
 *    $Revision: 31493 $
 **************************************************************************/

#include <intrinsics.h>
#include "arm_comm.h"

#ifndef __ARM922T_CP15_DRV_H
#define __ARM922T_CP15_DRV_H

#define NON_CACHABLE_ADDR     0xFFFFFFFC

#define L1_ENTRIES_NUMB       4096
#define L2_CP_ENTRIES_NUMB    256
#define L2_FP_ENTRIES_NUMB    1024


#define TSB_INVALID { 0, 0, 0, 0 }
#define TTB_INVALID { 0, TableInvalid }

#define L1_COARSES_PAGE_ENTRY(Numb, VirtAddr ,PhAddr, Domain) \
  { Numb, VirtAddr, PhAddr, ((Domain << 5) | TtL1CoarsePage)}
#define L1_SECTIONS_ENTRY(Numb, VirtAddr ,PhAddr, AP, Domain,C,B) \
  { Numb, VirtAddr, PhAddr, ((AP << 10) | (Domain << 5) | (C << 3) | (B << 2) | TtL1Section)}
#define L1_FINE_PAGE_ENTRY(Numb, VirtAddr ,PhAddr, Domain) \
  { Numb, VirtAddr, PhAddr, ((Domain << 5) | TtL1FinePage)}

#define L2_CT_LARGE_PAGE_ENTRY(Numb, VirtAddr ,PhAddr, Ap0, Ap1, Ap2, Ap3,C,B) \
  { Numb*16, VirtAddr, PhAddr, \
  ((Ap3 << 10) | (Ap2 << 8) | (Ap1 << 6) | (Ap0 << 4) | (C << 3) | (B << 2) |  \
    TtL2LargePage)}
#define L2_CT_SMALL_PAGE_ENTRY(Numb, VirtAddr ,PhAddr, Ap0, Ap1, Ap2, Ap3,C,B) \
  { Numb, VirtAddr, PhAddr, \
  ((Ap3 << 10) | (Ap2 << 8) | (Ap1 << 6) | (Ap0 << 4) | (C << 3) | (B << 2) |  \
    TtL2SmallPage)}

#define L2_FT_LARGE_PAGE_ENTRY(Numb, VirtAddr ,PhAddr, Ap0, Ap1, Ap2, Ap3,C,B) \
  { Numb*64, VirtAddr, PhAddr, \
  ((Ap3 << 10) | (Ap2 << 8) | (Ap1 << 6) | (Ap0 << 4) | (C << 3) | (B << 2) |  \
    TtL2LargePage)}
#define L2_FT_SMALL_PAGE_ENTRY(Numb, VirtAddr ,PhAddr, Ap0, Ap1, Ap2, Ap3,C,B) \
  { Numb*4, VirtAddr, PhAddr, \
  ((Ap3 << 10) | (Ap2 << 8) | (Ap1 << 6) | (Ap0 << 4) | (C << 3) | (B << 2) |  \
    TtL2SmallPage)}
#define L2_FT_TINY_PAGE_ENTRY(Numb, VirtAddr ,PhAddr, Ap,C,B) \
  { Numb, VirtAddr, PhAddr, \
  ((Ap << 4) | (C << 3) | (B << 2) | \
    TtL2TinyPage)}

// CP15 Registers
// ID register
#define CP15_ID           0

// Control register
#define CP15_CTRL         1
// CP15 Control register bits
#define CP15_CTRL_M      (1UL <<  0)    // MMU enable/disable
#define CP15_CTRL_A      (1UL <<  1)    // Alignment fault enable/disable
#define CP15_CTRL_C      (1UL <<  2)    // DCache enable/disable
#define CP15_CTRL_B      (1UL <<  7)    // Big-endian/little-endian
#define CP15_CTRL_S      (1UL <<  8)    // System protection
#define CP15_CTRL_R      (1UL <<  9)    // ROM protection
#define CP15_CTRL_I      (1UL << 12)    // ICache enable/disable
#define CP15_CTRL_V      (1UL << 13)    // Location of exception vectors
#define CP15_CTRL_RR     (1UL << 14)    // Round robin/random replacement
#define CP15_CTRL_L4     (1UL << 15)    // Determines if the T bit is set when load instructions change the PC

// Translation table base address (alignment 4KB)
#define CP15_TTB_ADDR     2

// Domain access control register
#define CP15_DA_CTRL      3

#define CP15_DA_CTRL_D0(Val)  ((Val & 0x3) <<  0)
#define CP15_DA_CTRL_D1(Val)  ((Val & 0x3) <<  2)
#define CP15_DA_CTRL_D2(Val)  ((Val & 0x3) <<  4)
#define CP15_DA_CTRL_D3(Val)  ((Val & 0x3) <<  6)
#define CP15_DA_CTRL_D4(Val)  ((Val & 0x3) <<  8)
#define CP15_DA_CTRL_D5(Val)  ((Val & 0x3) << 10)
#define CP15_DA_CTRL_D6(Val)  ((Val & 0x3) << 12)
#define CP15_DA_CTRL_D7(Val)  ((Val & 0x3) << 14)
#define CP15_DA_CTRL_D8(Val)  ((Val & 0x3) << 16)
#define CP15_DA_CTRL_D9(Val)  ((Val & 0x3) << 18)
#define CP15_DA_CTRL_D10(Val) ((Val & 0x3) << 20)
#define CP15_DA_CTRL_D11(Val) ((Val & 0x3) << 22)
#define CP15_DA_CTRL_D12(Val) ((Val & 0x3) << 24)
#define CP15_DA_CTRL_D13(Val) ((Val & 0x3) << 25)
#define CP15_DA_CTRL_D14(Val) ((Val & 0x3) << 28)
#define CP15_DA_CTRL_D15(Val) ((Val & 0x3) << 30)

// CP15 fault status register
#define CP15_FAULT_STAT   5

// CP15 fault address register
#define CP15_FAULT_ADDR   6

// CP15 Cache operations
#define CP15_CACHE_OPR    7

// CP15 TLB operation
#define CP15_TBL_OPR      8

// CP15 Cache lockdown
#define CP15_C_LD         9

// CP15 TBL lockdown
#define CP15_TBL_LD       10

// CP15 Process identifier register
#define CP15_PROCESS_IDNF 13

// CP15 Test
#define CP15_TEST         15

typedef enum
{
   DomainNoAccess = 0, DomainClient, DomainManager = 3,
} MmuDomainType_t;

typedef enum
{
   TtL1Invalid = 0, TtL1CoarsePage, TtL1Section, TtL1FinePage,
} TtL1EntryType_t;

typedef enum
{
   TtL2Invalid = 0, TtL2LargePage, TtL2SmallPage, TtL2TinyPage,
} TtL2EntryType_t;

typedef enum
{
   TableInvalid = 0, TableL1, TableL2_Coarse, TableL2_Fine,
} TableType_t;

typedef enum
{
   PC15_FASTBUS_MODE = 0, PC15_SYNC_MODE, PC15_ASYNC_MODE = 3
} ClkMode_t;

#pragma pack(1)
typedef union _TtEntry_t
{
   Int32U Data;
   struct
   {
      Int32U Type     : 2;
      Int32U          : 3;
      Int32U Domain   : 4;
      Int32U          :23;
   };
   // CoarsePage
   struct
   {
      Int32U          :10;
      Int32U BaseAddr :22;
   } CoarsePage;
   // Section
   struct
   {
      Int32U          : 2;
      Int32U B        : 1;
      Int32U C        : 1;
      Int32U          : 6;
      Int32U BaseAddr :22;
   } Section;
   // FinePage
   struct
   {
      Int32U          :12;
      Int32U BaseAddr :20;
   } FinePage;
} TtEntry_t, *pTtEntry_t;

typedef struct _TtSectionBlock_t
{
   Int32U NubrOfSections;
   Int32U VirtAddr;
   Int32U PhysAddr;
   TtEntry_t Entry;
} TtSectionBlock_t, * pTtSectionBlock_t;

typedef struct _TtTableBlock_t
{
   pInt32U     TableAddr;
   TableType_t TableType;
} TtTableBlock_t, * pTtTableBlock_t;

#pragma pack()

#define TT_ENTRY_INVALID          0

#define TTL1_SECTION_PADDR_MASK   0xFFF00000
#define TTL1_CP_PADDR_MASK        0xFFFFFC00
#define TTL1_FP_PADDR_MASK        0xFFFFF000

#define TTL2_LP_PADDR_MASK        0xFFFF0000
#define TTL2_SP_PADDR_MASK        0xFFFFF000
#define TTL2_TP_PADDR_MASK        0xFFFFFC00

/*************************************************************************
 * Function Name: CP15_GetTtb
 * Parameters: none
 *
 * Return: Int32U
 *
 * Description: Function returning the TTB register
 *
 *************************************************************************/
__arm Int32U CP15_GetTtb (void);

/*************************************************************************
 * Function Name: CP15_GetStatus
 * Parameters: none
 *
 * Return: Int32U
 *
 * Description: Function returning the MMU control register
 *
 *************************************************************************/
__arm Int32U CP15_GetStatus (void);

/*************************************************************************
 * Function Name: CP15_GetDomain
 * Parameters: none
 *
 * Return: Int32U
 *
 * Description: Function returning the MMU domain access register
 *
 *************************************************************************/
__arm Int32U CP15_GetDomain (void);

/*************************************************************************
 * Function Name: CP15_SetDomain
 * Parameters: Int32U DomainAccess
 *
 * Return: Int32U
 *
 * Description: Function set the MMU domain access register
 *
 *************************************************************************/
__arm void CP15_SetDomain (Int32U DomainAccess);

/*************************************************************************
 * Function Name: CP15_InvalAllCache
 * Parameters: none
 *
 * Return: none
 *
 * Description: Invalidate entire cache
 *
 *************************************************************************/
__arm void CP15_InvalAllCache (void);

/*************************************************************************
 * Function Name: CP15_InvalAllTbl
 * Parameters: none
 *
 * Return: none
 *
 * Description: Invalidate TLB
 *
 *************************************************************************/
__arm void CP15_InvalAllTbl (void);

/*************************************************************************
 * Function Name: CP15_SetMmu
 * Parameters: Int32U Ctrl
 *
 * Return: none
 *
 * Description: Set CP15 control register
 *
 *************************************************************************/
__arm void CP15_SetMmu (Int32U Ctrl);

/*************************************************************************
 * Function Name: CP15_SetTtb
 * Parameters: pInt32U pTtb
 *
 * Return: none
 *
 * Description: Set CP15 TTB base address register
 *
 *************************************************************************/
__arm void CP15_SetTtb (pInt32U pTtb);

/*************************************************************************
 * Function Name: CP15_SetDac
 * Parameters: Int32U da
 *
 * Return: none
 *
 * Description: Set CP15 domain access register
 *
 *************************************************************************/
__arm void CP15_SetDac (Int32U da);

/*************************************************************************
 * Function Name: CP15_WriteBuffFlush
 * Parameters: none
 *
 * Return: none
 *
 * Description:  Flush the write buffer and wait for completion
 *              of the flush.
 *
 *************************************************************************/
__arm void CP15_WriteBuffFlush (void);

/*************************************************************************
 * Function Name: CP15_GetFaultStat
 * Parameters: none
 *
 * Return: Int32U
 *
 * Description: Function returning the MMU fault status register
 *
 *************************************************************************/
__arm Int32U CP15_GetFaultStat (void);

/*************************************************************************
 * Function Name: CP15_GetFaultAddr
 * Parameters: none
 *
 * Return: Int32U
 *
 * Description: Function returning the MMU fault address register
 *
 *************************************************************************/
__arm Int32U CP15_GetFaultAddr (void);

/*************************************************************************
 * Function Name: CP15_GetFcsePid
 * Parameters: none
 *
 * Return: Int32U
 *
 * Description: Function returning the MMU Process identifier
 *             FCSE PID register
 *
 *************************************************************************/
__arm Int32U CP15_GetFcsePid (void);

/*************************************************************************
 * Function Name: CP15_GetPraceProcId
 * Parameters: none
 *
 * Return: Int32U
 *
 * Description: Function returning the MMU Trace Process identifier
 *             register
 *
 *************************************************************************/
__arm Int32U CP15_GetPraceProcId (void);

/*************************************************************************
 * Function Name: CP15_SetFcsePid
 * Parameters: Int32U FcsePid
 *
 * Return: none
 *
 * Description: Function set the MMU Process identifier
 *             FCSE PID register
 *
 *************************************************************************/
__arm void CP15_SetFcsePid (Int32U FcsePid);

/*************************************************************************
 * Function Name: CP15_SetPraceProcId
 * Parameters: Int32U
 *
 * Return: none
 *
 * Description: Function set the MMU Trace Process identifier
 *             register
 *
 *************************************************************************/
__arm void CP15_SetPraceProcId (Int32U Trace);

/*************************************************************************
 * Function Name: CP15_WriteBuffFlush
 * Parameters: pTtSectionBlock_t pTtSB, pTtTableBlock_t pTtTB
 *
 * Return: Boolean
 *
 *  Return error if MMU is enabled. Return error if target
 * Translation Table address is not 16K aligned. Clear the
 * Translation Table area. Build the Translation Table from the
 * initialization data in the Section Block array. Return no error.
 *
 * Description:  Initializes the MMU tables.
 *
 *
 *************************************************************************/
Boolean CP15_InitMmuTtb(const TtSectionBlock_t * pTtSB,
                        const TtTableBlock_t * pTtTB);

/*************************************************************************
 * Function Name: CP15_Mmu
 * Parameters: Boolean Enable
 *
 * Return: none
 *
 * Description: Enable/Disable MMU
 *
 *************************************************************************/
void CP15_Mmu (Boolean Enable);

/*************************************************************************
 * Function Name: CP15_Cache
 * Parameters: Boolean Enable
 *
 * Return: none
 *
 * Description: Enable/Disable Cache
 *
 *************************************************************************/
void CP15_Cache (Boolean Enable);

/*************************************************************************
 * Function Name: CP15_ICache
 * Parameters: Boolean Enable
 *
 * Return: none
 *
 * Description: Enable/Disable I cache
 *
 *************************************************************************/
void CP15_ICache (Boolean Enable);

/*************************************************************************
 * Function Name: CP15_DCache
 * Parameters: Boolean Enable
 *
 * Return: none
 *
 * Description: Enable/Disable D cache
 *
 *************************************************************************/
void CP15_DCache (Boolean Enable);

/*************************************************************************
 * Function Name: CP15_SysProt
 * Parameters: Boolean Enable
 *
 * Return: none
 *
 * Description: Enable/Disable S cache
 *
 *************************************************************************/
void CP15_SysProt (Boolean Enable);

/*************************************************************************
 * Function Name: CP15_RomProt
 * Parameters: Boolean Enable
 *
 * Return: none
 *
 * Description: Enable/Disable ROM protection.
 *
 *************************************************************************/
void CP15_RomProt (Boolean Enable);

#endif // __ARM926EJ_CP15_DRV_H
