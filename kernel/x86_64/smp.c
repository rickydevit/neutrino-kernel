#include "smp.h"
#include "kernel/common/kservice.h"
#include "thirdparty/stivale2.h"

// === PRIVATE FUNCTIONS ========================



// === PUBLIC FUNCTIONS =========================

void init_smp(struct stivale2_struct_tag_smp *smp_struct) {
    ks.log("Initializing other CPUs...");
    ks.dbg("Found %i CPUs. x2APIC is %c", smp_struct->cpu_count, (smp_struct->flags == 1) ? "enabled" : "disabled");
}
