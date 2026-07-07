bits 64

extern handle_exception

global divideErrorException                 ; 0
global debugException                       ; 1
global breakpointException                  ; 3 (no 2)
global overflowException                    ; 4
global BOUNDRangeExceededException          ; 5
global invalidOpcodeException               ; 6
global deviceNotAvailableException          ; 7
global doubleFaultException                 ; 8
global coprocessorSegmentOverrunException   ; 9
global invalidTSSException                  ; 10
global segmentNotPresentException           ; 11
global stackSegmentFaultException           ; 12
global generalProtectionException           ; 13
global pageFaultException                   ; 14
global mathFaultException                   ; 16 (no 15)
global alignmentCheckException              ; 17
global machineCheckException                ; 18
global SIMDFloatingPointException           ; 19
global virtualizationException              ; 20
; 21-31 are reserved

divideErrorException:
    PUSH 0
    PUSH 0
    JMP handler
debugException:
    PUSH 0
    PUSH 1
    JMP handler
breakpointException:
    PUSH 0
    PUSH 3
    JMP handler
overflowException:
    PUSH 0
    PUSH 4
    JMP handler
BOUNDRangeExceededException:
    PUSH 0
    PUSH 5
    JMP handler
invalidOpcodeException:
    PUSH 0
    PUSH 6
    JMP handler
deviceNotAvailableException:
    PUSH 0
    PUSH 7
    JMP handler
doubleFaultException:
    PUSH 8
    JMP handler
coprocessorSegmentOverrunException:
    PUSH 0
    PUSH 9
    JMP handler
invalidTSSException:
    PUSH 10
    JMP handler
segmentNotPresentException:
    PUSH 11
    JMP handler
stackSegmentFaultException:
    PUSH 12
    JMP handler
generalProtectionException:
    PUSH 13
    JMP handler
pageFaultException:
    PUSH 14
    JMP handler
mathFaultException:
    PUSH 0
    PUSH 16
    JMP handler
alignmentCheckException:
    PUSH 17
    JMP handler
machineCheckException:
    PUSH 0
    PUSH 18
    JMP handler
SIMDFloatingPointException:
    PUSH 0
    PUSH 19
    JMP handler
virtualizationException:
    PUSH 0
    PUSH 20
    JMP handler


align 0x08, db 0x00
handler:
    PUSH RAX
    PUSH RBX
    PUSH RCX
    PUSH RDX
    PUSH RSI
    PUSH RDI
    PUSH RBP
    PUSH R8
    PUSH R9
    PUSH R10
    PUSH R11
    PUSH R12
    PUSH R13
    PUSH R14
    PUSH R15
    MOV RAX, CR2
    PUSH RAX
    CLD
    MOV RDI, RSP
    CALL handle_exception
    ADD RSP, 8 ; get rid of CR2 that we pushed
    POP R15
    POP R14
    POP R13
    POP R12
    POP R11
    POP R10
    POP R9
    POP R8
    POP RBP
    POP RDI
    POP RSI
    POP RDX
    POP RCX
    POP RBX
    POP RAX
    ADD RSP, 18 ; discard interrupt number and error code
    STI
    IRETQ