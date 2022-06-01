#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <neutrino/macros.h>

#define ELF_MAGIC "\177ELF"

// Elf headers

typedef enum __elf_mode {
    ELF_32BIT = 1,
    ELF_64BIT = 2
} packed ElfMode;

typedef enum __elf_endian {
    ELF_LITTLE_ENDIAN = 1,
    ELF_BIG_ENDIAN = 2
} packed ElfEndian;

typedef enum __elf_type {
    _TYPE_FORCE_16_BIT = 0xffff,
    ELF_RELOCATABLE = 1,
    ELF_EXECUTABLE = 2,
    ELF_SHARED = 3,
    ELF_CORE = 4,
} packed ElfType;

typedef enum __elf_arch {
    _ARCH_FORCE_16_BIT = 0xffff,
    ELF_NO_SPECIFIC = 0,
    ELF_SPARC = 2,
    ELF_x86 = 3,
    ELF_MIPS = 8,
    ELF_POWERPC = 0x14,
    ELF_ARM = 0x28,
    ELF_SUPERH = 0x2a,
    ELF_IA64 = 0x32,
    ELF_x86_64 = 0x3e,
    ELF_AARCH64 = 0xb7,
    ELF_RISC_V = 0xf3
} packed ElfArch;

typedef struct __elf_header_common {
    char magic[4];
    ElfMode elf_mode;
    ElfEndian endian_type;
    uint8_t header_version;
    uint8_t os;
    uint8_t abi_version;
    uint8_t _padding[7];
} packed ElfCommonHeader;

typedef struct __elf_64_header {
    ElfCommonHeader common;
    ElfType elf_type;
    ElfArch elf_arch;
    uint32_t elf_version;
    
    uintptr_t entry_point;
    uintptr_t program_offset;
    uintptr_t sections_offset;

    uint32_t flags;
    uint16_t size;

    uint16_t programs_size;
    uint16_t programs_count;

    uint16_t sections_size;
    uint16_t sections_count;

    uint16_t strings_section_index;
} packed Elf64Header;

// Elf program header

typedef enum __elf_segment_type {
    _SEGTYPE_FORCE_32_BIT = 0xffffffff,
    SEGMENT_TYPE_NULL = 0,
    SEGMENT_TYPE_LOAD = 1,
    SEGMENT_TYPE_DYNAMIC = 2,
    SEGMENT_TYPE_INTERP = 3,
    SEGMENT_TYPE_NOTE = 4
} ElfSegmentType;

typedef enum __elf_segment_flags {
    _SEGFLAGS_FORCE_32_BIT = 0xffffffff,
    SEGMENT_FLAGS_EXECUTABLE = 0x1,
    SEGMENT_FLAGS_WRITABLE = 0x2,
    SEGMENT_FLAGS_READABLE = 0x4
} ElfSegmentFlags;

typedef struct __elf_64_program_header {
    ElfSegmentType type;
    ElfSegmentFlags flags;
    
    uintptr_t file_offset;
    uintptr_t vaddr;
    uint64_t _undefined;

    uint64_t file_size;
    uint64_t mem_size;
    uint64_t required_alignment;
} packed Elf64ProgramHeader;

// Elf section header

typedef struct __elf_64_section_header {
    uint32_t name;
    uint32_t type;
    uint64_t flags;

    uintptr_t vaddr;
    uintptr_t file_offset;
    uint64_t file_size;
    uint32_t link;
    uint32_t info;
    uint64_t addr_align;
    uint64_t entry_size;
} packed Elf64SectionHeader;

// Functions

bool elf_check(const Elf64Header* header);
