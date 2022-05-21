#include "elf.h" 
#include <string.h>
#include <stdbool.h>

// === PRIVATE FUNCTIONS ========================

bool elf_check(const Elf64Header* header) {
    return (strncmp(header->common.magic, ELF_MAGIC, 4) == 0);
}

// === PUBLIC FUNCTIONS =========================

void elf_load(const Elf64Header* header) {

}
