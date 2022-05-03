#include "modules.h"
#include "stivale2.h"
#include <stdint.h>
#include <_null.h>
#include <string.h>
#include "kernel/common/kservice.h"

static size_t modules_count = 0;
static BootModule modules[MAX_MODULES];

// === PRIVATE FUNCTIONS ========================

// === PUBLIC FUNCTIONS =========================

int init_modules(uintptr_t addr) {
    struct stivale2_struct_tag_modules* m = (struct stivale2_struct_tag_modules*)addr;
    
    ks.log("Initializing limine modules... found %u modules:", m->module_count);
    modules_count = (size_t)m->module_count;

    for (size_t i = 0; i < modules_count; i++) {
        modules[i] = (BootModule){
            {0}, 
            m->modules[i].begin, 
            (size_t)(m->modules[i].end-m->modules[i].begin)
        };
        strcpy(m->modules[i].string, modules[i].name);

        ks.log(" > module \"%c\" begin at %x length %u", modules[i].name, modules[i].start_addr, modules[i].length);
    }

    ks.log("Modules initialized");
    return modules_count;
}

BootModule* module_get_by_name(const char* name) {
    for (size_t i = 0; i < modules_count; i++) {
        if (strcmp(modules[i].name, name) != 0) continue;
        else return &modules[i];
    }

    return nullptr;
}
