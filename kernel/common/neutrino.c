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
#include <liballoc.h>
#include <neutrino-gfx/tga.h>
#include <neutrino/syscall.h>

// === PRIVATE FUNCTIONS ========================

void display() {
    ks.dbg("Initializing video driver...");

    int i = 0;
    struct __dirent* node = 0;

    while ((node = fs_readdir(root, i)) != nullptr) {
        FsNode* fsnode = fs_finddir(root, node->name);

        if ((fsnode->flags & 0x7) == FS_FILE) {
            char* buf = (char*)kmalloc(fsnode->length);
            fs_read(fsnode, 0, fsnode->length, (uint8_t*)buf);

            // logo check (temp splashscreen)
            TGAHeader* tga_h = (TGAHeader*)buf;
            if (strcmp(fsnode->name, "logo.tga")) {
                uint8_t* pixels = (uint8_t*)((uint8_t*)tga_h) + sizeof(TGAHeader);
                if (init_bga()) {
                    DisplayInfo info = bga_get_display_info();
                    ks.dbg("BGA driver initialized. Framebuffer at %x", info.lbf);
                    
                    if (!init_video_driver(info.lbf, info.width, info.height, info.pitch, info.bpp)) return;

                    // ? temporary splashscreen
                    // draw_rect(info.width/2-90, info.height/2-90, 180, 180, (Color){0,44,35,219});
                    
                    uint8_t* temp = pixels;
                    for (size_t y = 0; y < 150; y++)
                        for (size_t x = 0; x < 150; x++, temp+=4)
                            put_pixel(565 + x, 285 + y, (Color){temp[3], temp[2], temp[1], temp[0]});
                }
            }
            
            kfree(buf);
        }
        i++;
    }
}

void initrd_explorer() {
    int i = 0;
    struct __dirent* node = 0;

    while ((node = fs_readdir(root, i)) != nullptr) {
        FsNode* fsnode = fs_finddir(root, node->name);

        if ((fsnode->flags & 0x7) == FS_FILE) {
            char* buf = (char*)kmalloc(fsnode->length);
            fs_read(fsnode, 0, fsnode->length, (uint8_t*)buf);

            // elf check
            const Elf64Header* header = (const Elf64Header*)buf;
            if (elf_check(header)) 
                load_binary((const uintptr_t)header, fsnode->name, true);
            
            kfree(buf);
        }
        i++;
    }
}

// === PUBLIC FUNCTIONS =========================

void neutrino_main() {
    uintptr_t initrd = module_get_by_name("initrd")->start_addr;
    if (initrd != nullptr) root = init_initrd(initrd);

    init_pci();
    
    // Task* initrd_task = NewTask("initrd_explorer", false);
    // sched_start(initrd_task, (uintptr_t)initrd_explorer);

    Task* display_task = NewTask("display", false);
    sched_start(display_task, (uintptr_t)display);
    
    arch_idle();
}
