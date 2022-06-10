#pragma once
#include <stdint.h>
#include <size_t.h>
#include <stdbool.h>

#define AGENT_NAME_MAX 32

typedef uint64_t AgentID;

typedef enum __ipc_type {
    IPC_SEND,
    IPC_RECEIVE,
    IPC_PEEK,
    IPC_BROADCAST,
} IpcType;

typedef struct __agent {
    AgentID id;
    char name[AGENT_NAME_MAX+1];
} Agent;

typedef struct __package {
    Agent* sender;
    size_t size;
    uintptr_t buffer;
} Package;

void agent_init(Agent* agent, const char* name);
Package* NewPackage(Agent* agent, uintptr_t data, size_t size);
void DestroyPackage(Package* pack);
