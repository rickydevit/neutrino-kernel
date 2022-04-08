#pragma once

#include <neutrino/macros.h>
#include <neutrino/lock.h>
#include <liballoc.h>
#include "mem_virt.h"
#include "kernel/common/memory/space.h"
#include "kernel/common/memory/memory.h"

struct __space {
    Lock lock;
    PageTable* page_table;

    MemoryRangeNode* memory_ranges;
};
