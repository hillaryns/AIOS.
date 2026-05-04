# ⚡ AIOS — Development Roadmap

> **Vision:** Build a bare-metal Operating System from scratch that runs a local LLM as its core logic engine—eliminating reliance on external APIs and creating a truly autonomous, intelligent computer system.

---

## Current State (Baseline)

| Area | Status |
|------|--------|
| Development Environment (Cross-Compiler) | ✅ Complete |
| Bootloader (16-bit Real Mode entry) | ✅ Complete |
| Kernel Core (GDT, IDT, Long Mode) | ✅ Complete |
| Memory Management (Physical/Virtual) | ⬜ Not Started |
| AI Integration (Local Inference) | ⬜ Not Started |

---

## Phase 1 — The Foundation (Boot & Architecture) ✅ COMPLETE (v0.1.0)

**Goal:** Transition the CPU from legacy 16-bit mode to modern 64-bit Long Mode and establish a minimal execution environment.

### 1.1 — Toolchain & Environment ✅

- ✅ Build `x86_64-elf` cross-compiler (GCC/Binutils) to target bare metal
- ✅ Configure QEMU emulator for rapid testing & debugging
- ✅ Set up `gdb` debugging scripts for real-time CPU state inspection
- ✅ Create build system (Makefile/CMake) to link Assembly and C code

### 1.2 — Bootloader Mechanics ✅

- ✅ Write Stage 1 Bootloader (MBR) in Assembly (fits in 512 bytes)
- ✅ Implement BIOS interrupts for basic disk reading
- ✅ Enable the A20 Line to access high memory
- ✅ Switch CPU from 16-bit Real Mode → 32-bit Protected Mode
- ✅ **Advanced:** Setup Paging Tables (PML4) to switch into 64-bit Long Mode

### 1.3 — Kernel Entry ✅

- ✅ Write `kernel_entry.asm` (stack setup, GDT loading)
- ✅ Link assembly entry point to C `kernel_main()` function
- ✅ Implement Basic VGA Text Driver (writing directly to `0xB8000` memory)
- ✅ Handle CPU exceptions (Double Fault, General Protection Fault)

> **Exit criteria:** ✅ OS boots from a `.iso`/`.img` file, switches to 64-bit mode, and prints `"AIOS Initialized"` to the screen without crashing.

---

## Phase 2 — Core Kernel Systems

**Goal:** Establish the "nervous system" of the OS—handling memory, interrupts, and basic hardware interaction.

### 2.1 — Memory Management (The Sandbox)

- ⬜ Detect available physical memory (via Multiboot2 info or BIOS)
- ⬜ Implement Physical Memory Manager (Bitmap Allocator)
- ⬜ Implement Virtual Memory Manager (Paging, 4KB & 2MB pages)
- ⬜ Write Kernel Heap Allocator (`kmalloc` / `kfree`)
- ⬜ **AIOS Specific:** Implement "Tensor Allocator" for contiguous memory blocks (essential for Matrix operations)

### 2.2 — Interrupts & Hardware

- ✅ Remap PIC (Programmable Interrupt Controller)
- ✅ Set up IDT (Interrupt Descriptor Table) with ISRs (Interrupt Service Routines)
- ⬜ Write PS/2 Keyboard Driver (Scan code to ASCII mapping)
- ⬜ Configure PIT (Programmable Interval Timer) for system clock
- ⬜ **AIOS Specific:** Setup APIC (Advanced PIC) for Multi-Core support (SMP) for parallel AI tasks

### 2.3 — Basic Drivers

- ⬜ PCI Enumeration (scanning hardware bus)
- ⬜ Basic Disk Driver (AHCI SATA or IDE) to read sectors
- ⬜ Serial Port Driver (for logging/debugging without screen)

> **Exit criteria:** Kernel successfully allocates memory, handles keyboard input, and manages system time.

---

## Phase 3 — Storage & Filesystem

**Goal:** Create a system to store and retrieve the massive binary weight files required by AI models.

### 3.1 — Filesystem Implementation

- ⬜ Implement a simple filesystem driver (FAT16/32 or custom "AI-FS")
- ⬜ Write `open`, `read`, `write`, `close` system call implementations
- ⬜ Create a Virtual File System (VFS) layer for abstraction
- ⬜ **AIOS Specific:** Design "Tensor Partition" format for streaming large model weights directly into RAM

### 3.2 — Persistence

- ⬜ Write MBR partition table to disk image
- ⬜ Implement file ownership and basic metadata
- ⬜ Create a basic shell command to list files (`ls` equivalent)

> **Exit criteria:** OS can read a file from the disk and load it into memory.

---

## Phase 4 — Multitasking & Scheduling

**Goal:** Enable the OS to manage multiple processes—crucial for running the AI engine alongside the UI.

### 4.1 — Process Management

- ⬜ Define Process Control Block (PCB) structure
- ⬜ Implement Context Switching (saving/restoring registers)
- ⬜ Write a Round-Robin Scheduler
- ⬜ Implement Kernel Threads (for background tasks)

### 4.2 — User Space & Safety

- ⬜ Implement Ring 3 (User Mode) switching
- ⬜ Setup TSS (Task State Segment) for privilege switching
- ⬜ Implement System Calls (Software Interrupts or `SYSCALL` instruction)
- ⬜ **AIOS Specific:** Create `sys_run_inference` system call

### 4.3 — The "Intelligent" Scheduler

- ⬜ **AIOS Specific:** Implement Priority Queue for responsiveness
- ⬜ **AIOS Specific:** Detect heavy compute tasks (Matrix Mult) and schedule efficiently (prevent UI freeze during LLM thinking)

> **Exit criteria:** OS can run multiple programs simultaneously and switch between them smoothly.

---

## Phase 5 — The AI Layer (The Brain)

**Goal:** Integrate the math engine and local LLM architecture directly into the kernel space.

### 5.1 — High-Performance Math

- ⬜ Enable FPU/SSE/AVX states in Context Switching
- ⬜ Write Matrix Multiplication kernel in Assembly (using AVX2/AVX-512)
- ⬜ Implement `matmul`, `vector_add`, `softmax` functions in Kernel Space
- ⬜ **AIOS Specific:** Optimize memory layout for cache locality (Cache Blocking)

### 5.2 — Inference Engine

- ⬜ Port a tokenizer (BPE/Unigram) implementation to C (no stdlib)
- ⬜ Write a parser for model weight files (e.g., GGUF or SafeTensors format)
- ⬜ Implement the Transformer architecture (Attention, Feed Forward, Normalization)
- ⬜ **AIOS Specific:** Load a small pre-trained model (e.g., TinyLlama or a small RNN) into kernel memory

### 5.3 — Local LLM Integration

- ⬜ Create the AI Interface: `aios_query(prompt_buffer, output_buffer)`
- ⬜ Implement "Token Streaming" output (OS prints tokens as they generate)
- ⬜ Verify local inference runs entirely offline (no internet calls)

> **Exit criteria:** The OS can load a model file and generate text locally using native CPU instructions.

---

## Phase 6 — User Interface & Shell

**Goal:** Create the human-computer interface where the user interacts with the local LLM.

### 6.1 — Graphical Interface

- ⬜ Request Linear Frame Buffer (LFB) from bootloader (GUI mode)
- ⬜ Write basic graphics driver (draw pixel, line, rectangle)
- ⬜ Implement bitmap font rendering (text on graphics mode)

### 6.2 — The NLP Shell

- ⬜ Build a command-line interface on top of graphics driver
- ⬜ Implement text input buffer and cursor
- ⬜ **AIOS Specific:** Route user commands through the local LLM
- ⬜ **AIOS Specific:** Implement intent parsing: `"Open file X"` → LLM → System Call `open("X")`

> **Exit criteria:** A graphical or text-based terminal where the user can chat with the OS to execute commands.

---

## Milestone Summary

| Phase | Name | Key Deliverable | Target | Status |
|-------|------|-----------------|--------|--------|
| 1 | Foundation | Bootloader + Long Mode + Kernel Main | v0.1.0 | ✅ Done |
| 2 | Core Systems | Memory Manager + Interrupts + Keyboard | v0.2.0 | 🔄 Next |
| 3 | Storage | File System + Disk Drivers | v0.3.0 | ⬜ |
| 4 | Multitasking | Scheduler + User Mode + Syscalls | v0.5.0 | ⬜ |
| 5 | AI Layer | AVX Math + Transformer + Local Inference | v0.8.0 | ⬜ |
| 6 | Interaction | GUI + NLP Shell | v1.0.0 | ⬜ |
