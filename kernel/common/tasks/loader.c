#include "loader.h"
#include "task.h"
#include "kernel/common/memory/space.h"
#include "kernel/common/memory/memory.h"
#include "kernel/common/fs/fs.h"
#include "kernel/common/kservice.h"
#include "kernel/common/tasks/scheduler.h"
#include <elf/elf.h>
#include <stdint.h>
#include <stdbool.h>
#include <size_t.h>
#include <neutrino/macros.h>

// === PRIVATE FUNCTIONS ========================

void load_elf(const Elf64Header* header, const uintptr_t binary, Task* task) {
    // loading program headers
    Elf64ProgramHeader* prg_header = (Elf64ProgramHeader*)((uintptr_t)header + header->program_offset);
    for (size_t i = 0; i < header->programs_count; i++) {
        if (prg_header->type != SEGMENT_TYPE_LOAD) {
            prg_header = (Elf64ProgramHeader*)((uintptr_t)prg_header + header->programs_size);
            continue;
        }
        
        size_t size = AlignUp(Max(prg_header->mem_size, prg_header->file_size), PAGE_SIZE);
        VirtualMapping vmap = memory_allocate(size);
        
        memory_set((uint8_t*)vmap.virtual_base, 0, vmap.physical.size);
        memory_copy((uint8_t*)(binary + prg_header->file_offset), (uint8_t*)vmap.virtual_base, prg_header->file_size);

        if (!(prg_header->flags & SEGMENT_FLAGS_WRITABLE) && prg_header->file_size == prg_header->mem_size) {
            ks.dbg("ELF LOADING: loading program into task memory at %x (size %u, source %x, US)", 
                prg_header->vaddr, size, (binary + prg_header->file_offset));
            space_map(task->space, vmap.physical.base, prg_header->vaddr, size, ((task->user) ? MAP_USER : 0));
        } else {
            ks.dbg("ELF LOADING: loading program into task memory at %x (size %u, source %x, US-WR)", 
                prg_header->vaddr, size, (binary + prg_header->file_offset));
            space_map(task->space, vmap.physical.base, prg_header->vaddr, size, ((task->user) ? MAP_USER : 0) | MAP_WRITABLE);
        }

        memory_free(vmap);
        prg_header = (Elf64ProgramHeader*)((uintptr_t)prg_header + header->programs_size);
    }
}

// === PUBLIC FUNCTIONS =========================

void unoptimized load_binary(const uintptr_t binary, char* binary_name, bool user) {
    // check for ELF and validate
    if (elf_check((const Elf64Header*)binary)) {
        Elf64Header* h = (Elf64Header*)binary;

        if (h->common.elf_mode != ELF_64BIT || h->common.endian_type != ELF_LITTLE_ENDIAN) {
            ks.err("Unsupported ELF format (%c bit, %c endian)", 
                   (h->common.elf_mode == ELF_64BIT) ? "64" : "32", 
                   (h->common.endian_type == ELF_LITTLE_ENDIAN) ? "little" : "big");
            return;
        }

        ks.log("Loading ELF \"%c\"...", binary_name);
        Task* elf_task = NewTask(binary_name, user);
        load_elf(h, binary, elf_task);
        sched_start(elf_task, h->entry_point);

    } else {
        ks.err("Binary is not supported");
    }
}
