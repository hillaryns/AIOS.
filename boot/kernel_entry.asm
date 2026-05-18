; ============================================================
; AIOS — Kernel Entry (Multiboot2 → Long Mode, FIXED)
; ============================================================

bits 32

section .multiboot2
align 8
    dd 0xE85250D6
    dd 0
    dd header_end - header_start
    dd -(0xE85250D6 + 0 + (header_end - header_start))
header_start:
    dw 0
    dw 0
    dd 8
header_end:

section .text
global kernel_entry
extern kernel_main

kernel_entry:
    cli

    ; -------------------------------
    ; setup stack
    ; -------------------------------
    mov esp, stack_top

    ; -------------------------------
    ; setup paging (identity map 2MB)
    ; -------------------------------

    ; PML4 → PDPT
    mov eax, pdpt_table
    or eax, 0b11
    mov [pml4_table], eax

    ; PDPT → PD
    mov eax, pd_table
    or eax, 0b11
    mov [pdpt_table], eax

    ; PD entry (2MB page)
    mov eax, 0x00000083      ; present + rw + huge page
    mov [pd_table], eax

    ; -------------------------------
    ; enable long mode
    ; -------------------------------

    mov eax, pml4_table
    mov cr3, eax

    mov eax, cr4
    or eax, 1 << 5           ; enable PAE
    mov cr4, eax

    mov ecx, 0xC0000080      ; EFER
    rdmsr
    or eax, 1 << 8           ; LME
    wrmsr

    mov eax, cr0
    or eax, 1 << 31          ; paging
    mov cr0, eax

    ; -------------------------------
    ; load GDT and jump to 64-bit
    ; -------------------------------
    lgdt [gdt64.pointer]
    jmp 0x08:long_mode_entry


; ============================================================
; 64-bit code
; ============================================================
bits 64

long_mode_entry:
    mov rsp, stack_top
   ; pass multiboot magic + addr
   mov edi, eax
   mov rsi, rbx

    call kernel_main

.hang:
    cli
    hlt
    jmp .hang


; ============================================================
; paging structures
; ============================================================
section .bss
align 4096

pml4_table:
    resq 512

pdpt_table:
    resq 512

pd_table:
    resq 512


; ============================================================
; GDT (64-bit)
; ============================================================
section .rodata

gdt64:
    dq 0
    dq 0x00AF9A000000FFFF   ; code segment
    dq 0x00AF92000000FFFF   ; data segment

.pointer:
    dw $ - gdt64 - 1
    dq gdt64


; ============================================================
; stack
; ============================================================
section .bss
align 16

stack_bottom:
    resb 16384
stack_top:
