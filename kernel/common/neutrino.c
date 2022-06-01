#include "neutrino.h"
#include "arch.h"
#include "device/serial.h"
#include "video/display.h"
#include "kservice.h"
#include "tasks/scheduler.h"
#include "modules.h"
#include "fs/fs.h"
#include "fs/initrd.h"
#include "tasks/loader.h"
#include <libs/elf/elf.h>
#include <neutrino/syscall.h>

// === PRIVATE FUNCTIONS ========================

void cpu_test1() {
    sys_test(nullptr);
}

void cpu_test2() {
    ks.dbg("test from process 2");
}

void cpu_test3() {
    int i = 0;
    struct __dirent* node = 0;

    while ((node = fs_readdir(root, i)) != nullptr) {
    ks.dbg("Found file %c", node->name);
    FsNode* fsnode = fs_finddir(root, node->name);

    if ((fsnode->flags & 0x7) == FS_DIRECTORY)
        ks.dbg("\t(directory)");
    else {
        char* buf = (char*)kmalloc(fsnode->length);
        size_t sz = fs_read(fsnode, 0, fsnode->length, (uint8_t*)buf);
        const Elf64Header* header = (const Elf64Header*)buf;

        if (elf_check(header)) {
            ks.dbg("\t(ELF file) %u/%u", sz, fsnode->length);
            load_binary((const uintptr_t)header, fsnode->name, true);
        } else {
            ks.dbg("\t(file)");
        }
    }
    i++;
    }
}

// === PUBLIC FUNCTIONS =========================

void neutrino_main() {
    uintptr_t initrd = module_get_by_name("initrd")->start_addr;
    if (initrd != nullptr) root = init_initrd(initrd);

    // test scheduler
    Task* test = NewTask("test1", false);
    Task* test2 = NewTask("test2", false);
    Task* test3 = NewTask("test3", false);
    sched_start(test, (uintptr_t)cpu_test1);
    sched_start(test2, (uintptr_t)cpu_test2);
    sched_start(test3, (uintptr_t)cpu_test3);

    arch_idle();
}
