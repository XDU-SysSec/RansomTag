;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; LPC313x vector table
;;
;;
;; Copyright 2009 IAR Systems. All rights reserved.
;;
;; $Revision: 31493 $
;;


        MODULE  ?vector
        
; This module provides low level interrupt vector table.
; This table shuold be copyed or linked at virtula address 0x0
;
        SECTION .text:CODE:NOROOT(2)
     
        PUBLIC  __vector
        PUBLIC  __vector_end
        EXTERN  __iar_program_start
        EXTERN IRQ_Handler

        ARM ; 	
__vector:
__reset
  ldr pc,[pc,#24] ; Absolute jump can reach 4 GByte
__undef_handler	
  ldr pc,[pc,#24] ; Branch to undef_handler
__swi_handler	
  ldr pc,[pc,#24] ; Branch to swi_handler
__prefetch_handler
  ldr pc,[pc,#24] ; Branch to prefetch_handler
__data_handler	
  ldr pc,[pc,#24] ; Branch to data_handler
  dc32  0         ; RESERVED
__irq_handler
  ldr pc,[pc,#20] ; Branch to irq_handler
__fiq_handler	
  ldr pc,[pc,#20] ; Branch to fiq_handler
  ; Constant table entries (for ldr pc)
  dc32  __iar_program_start
  dc32 __undef_handler
  dc32 __swi_handler
  dc32 __prefetch_handler
  dc32 __data_handler
  dc32 IRQ_Handler
__vector_end
  dc32 __fiq_handler
  
        END
  
  
