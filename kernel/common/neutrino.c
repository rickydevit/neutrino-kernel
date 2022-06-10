#include "neutrino.h"
#include "arch.h"
#include "device/serial.h"
#include "device/pci/pci.h"
#include "video/display.h"
#include "video/bga.h"
#include "kservice.h"
#include "tasks/scheduler.h"
#include "modules.h"
#include "fs/fs.h"
#include "fs/initrd.h"
#include "tasks/loader.h"
#include <libs/elf/elf.h>
#include <neutrino/syscall.h>

// === PRIVATE FUNCTIONS ========================

void initrd_explorer() {
    int i = 0;
    struct __dirent* node = 0;

    while ((node = fs_readdir(root, i)) != nullptr) {
        FsNode* fsnode = fs_finddir(root, node->name);

        if ((fsnode->flags & 0x7) == FS_FILE ) {
            char* buf = (char*)kmalloc(fsnode->length);
            fs_read(fsnode, 0, fsnode->length, (uint8_t*)buf);
            const Elf64Header* header = (const Elf64Header*)buf;

            if (elf_check(header)) 
                load_binary((const uintptr_t)header, fsnode->name, true);
            
            kfree(buf);
        }
        i++;
    }
}

void display() {
    ks.dbg("Initializing video driver...");
    
    if (init_bga()) {
        DisplayInfo info = bga_get_display_info();
        ks.dbg("BGA driver initialized. Framebuffer at %x", info.lbf);
        
        init_video_driver(info.lbf, info.width, info.height, info.pitch, info.bpp);
        put_pixel(info.width/2, info.height/2, (Color){255,0,0});
    }
}

// === PUBLIC FUNCTIONS =========================

void neutrino_main() {
    uintptr_t initrd = module_get_by_name("initrd")->start_addr;
    if (initrd != nullptr) root = init_initrd(initrd);

    Task* test3 = NewTask("initrd_explorer", false);
    sched_start(test3, (uintptr_t)initrd_explorer);

    init_pci();
    display();
    
    arch_idle();
}
