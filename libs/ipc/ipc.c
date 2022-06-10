#include "ipc.h"
#include <stdbool.h>
#include <string.h>
#include <_null.h>
#include <size_t.h>
#include <stdint.h>
#include <linkedlist.h>
#include <liballoc.h>
#include <neutrino/lock.h>
#include <neutrino/macros.h>

static Lock agent_lock = NewLock;
static AgentID _global_agent_id = 0;

// === PRIVATE FUNCTIONS ========================

// === PUBLIC FUNCTIONS =========================

void agent_init(Agent* agent, const char* name) {
    LockOperation(agent_lock, agent->id = _global_agent_id++);
    memset((uint8_t*)agent->name, 0, AGENT_NAME_MAX+1);
    
    if (name != nullptr && strlen(name) > 0) 
        memcpy((uint8_t*)name, (uint8_t*)agent->name, Min(strlen(name), AGENT_NAME_MAX));
}

Package* NewPackage(Agent* agent, uintptr_t data, size_t size) {
    Package* pack = (Package*)lmalloc(sizeof(Package));
    pack->sender = agent;
    pack->size = size;
    pack->buffer = (uintptr_t)lmalloc(size);
    memcpy((uint8_t*)data, (uint8_t*)pack->buffer, size);

    return pack;
}

void DestroyPackage(Package* pack) {
    lfree((void*)pack->buffer);
    lfree(pack);
}
