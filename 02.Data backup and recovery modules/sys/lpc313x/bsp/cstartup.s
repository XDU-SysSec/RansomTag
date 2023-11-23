;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Part one of the system initialization code,
;; contains low-level
;; initialization.
;;
;; Copyright 2006 IAR Systems. All rights reserved.
;;
;; $Revision: 31493 $
;;

        MODULE  ?cstartup

        ;; Forward declaration of sections.
        SECTION IRQ_STACK:DATA:NOROOT(3)
        SECTION FIQ_STACK:DATA:NOROOT(3)
        SECTION SVC_STACK:DATA:NOROOT(3)
        SECTION ABT_STACK:DATA:NOROOT(3)
        SECTION UND_STACK:DATA:NOROOT(3)
        SECTION CSTACK:DATA:NOROOT(3)
        SECTION BTLD_STACK:DATA:NOROOT(3)
        SECTION CODE_END:DATA

;
; The module in this file are included in the libraries, and may be
; replaced by any user-defined modules that define the PUBLIC symbol
; __iar_program_start or a user defined start symbol.
;
; To override the cstartup defined in the library, simply add your
; modified version to the workbench project.



        SECTION .bootheader:CODE:ROOT(2)

        PUBLIC  __iar_program_start
        ARM	  ; Always ARM mode after reset

; This initial handler is only for reset, a real application will
; replace this exception handler in memory with a more capable one
;__vector
    B		__iar_program_start  ; Reset
	DCD		0x41676d69		       ;image magic number
	DCD		0,0,0,0,0
image_type
	DCD		0x0000000A		       ;Plain text no CRC check

sizeOfPermanentCode
  DCD 	sfe(CODE_END)-0x11029000 ;
	DCD 	0,0
bootparameter
	DCD 	0
	DCD 	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

        PUBLIC  ?cstartup
        EXTERN  ?main
        EXTERN  bootloader

        SECTION .text:CODE:NOROOT(2)
        ARM

__iar_program_start:
?cstartup:

;
; Add initialization needed before setup of stackpointers here.
;
CP_DIS_MASK DEFINE  0xFFFFEFFA
// Disable Addr translation, D cache and enable I cache
                mrc     p15,0,R1,C1,C0,0
                ldr     R0,=CP_DIS_MASK     ; 0xFFFFEFFA
                and     R1,R1,R0
                orr     R1,R1,#(1<<12)
                mcr     p15,0,R1,C1,C0,0

#ifdef BOOT_LEVEL_2
        mrs         r0,cpsr                       ; Original PSR value
        bic         r0,r0,#MODE_MSK               ; Clear the mode bits
        orr         r0,r0,#SVC_MODE               ; Set Supervisor mode bits
        msr         cpsr_c,r0                     ; Change the mode
        ldr         sp,=SFE(BTLD_STACK)           ; End of bootloader stack ()

        LDR     r0, =bootloader
        blx     r0
#endif

;
; Initialize the stack pointers.
; The pattern below can be used for any of the exception stacks:
; FIQ, IRQ, SVC, ABT, UND, SYS.
; The USR mode uses the same stack as SYS.
; The stack segments must be defined in the linker command file,
; and be declared above.
;
; --------------------
; Mode, correspords to bits 0-5 in CPSR

MODE_MSK DEFINE 0x1F            ; Bit mask for mode bits in CPSR
USR_MODE DEFINE 0x10            ; User mode
FIQ_MODE DEFINE 0x11            ; Fast Interrupt Request mode
IRQ_MODE DEFINE 0x12            ; Interrupt Request mode
SVC_MODE DEFINE 0x13            ; Supervisor mode
ABT_MODE DEFINE 0x17            ; Abort mode
UND_MODE DEFINE 0x1B            ; Undefined Instruction mode
SYS_MODE DEFINE 0x1F            ; System mode

        mrs         r0,cpsr                       ; Original PSR value
        bic         r0,r0,#MODE_MSK               ; Clear the mode bits
        orr         r0,r0,#SVC_MODE               ; Set Supervisor mode bits
        msr         cpsr_c,r0                     ; Change the mode
        ldr         sp,=SFE(SVC_STACK)            ; End of SVC_STACK

        bic         r0,r0,#MODE_MSK               ; Clear the mode bits
        orr         r0,r0,#UND_MODE               ; Set Undefined mode bits
        msr         cpsr_c,r0                     ; Change the mode
        ldr         sp,=SFE(UND_STACK)            ; End of UND_MODE

        bic         r0,r0,#MODE_MSK               ; Clear the mode bits
        orr         r0,r0,#ABT_MODE               ; Set Data abort mode bits
        msr         cpsr_c,r0                     ; Change the mode
        ldr         sp,=SFE(ABT_STACK)            ; End of ABT_STACK

        bic         r0,r0,#MODE_MSK               ; Clear the mode bits
        orr         r0,r0,#FIQ_MODE               ; Set FIR mode bits
        msr         cpsr_c,r0                     ; Change the mode
        ldr         sp,=SFE(FIQ_STACK)            ; End of FIQ_STACK

        bic         r0,r0,#MODE_MSK               ; Clear the mode bits
        orr         r0,r0,#IRQ_MODE               ; Set IRQ mode bits
        msr         cpsr_c,r0                     ; Change the mode
        ldr         sp,=SFE(IRQ_STACK)            ; End of IRQ_STACK

        bic         r0,r0,#MODE_MSK               ; Clear the mode bits
        orr         r0,r0,#SYS_MODE               ; Set System mode bits
        msr         cpsr_c,r0                     ; Change the mode
        ldr         sp,=SFE(CSTACK)               ; End of CSTACK

#ifdef __ARMVFP__
        ;; Enable the VFP coprocessor.

        MOV     r0, #0x40000000         ; Set EN bit in VFP
        FMXR    fpexc, r0               ; FPEXC, clear others.

;
; Disable underflow exceptions by setting flush to zero mode.
; For full IEEE 754 underflow compliance this code should be removed
; and the appropriate exception handler installed.
;

        MOV     r0, #0x01000000         ; Set FZ bit in VFP
        FMXR    fpscr, r0               ; FPSCR, clear others.
#endif

; Continue to ?main for C-level initialization.

        LDR     r0, =?main
        BX      r0

       SECTION CODE_END:DATA(9)
       DATA
CodeEnd:

    END


