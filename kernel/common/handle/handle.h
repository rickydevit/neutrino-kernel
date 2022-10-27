#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <neutrino/macros.h>

#include "../fs/fs.h"
#include "../tasks/task.h"

#define FOREACH_HANDLETYPE(c) \
    c(HANDLE_TYPE_FILE)  \

#define FOREACH_HANDLERESULT(c) \
    c(HANDLE_RESULT_SUCCESS) \
    c(HANDLE_RESULT_ERROR) \
    c(HANDLE_RESULT_BAD_REQUEST) \
    c(HANDLE_RESULT_ALREADY_OWNED) \

typedef enum __handle_type {
    FOREACH_HANDLETYPE(GenerateEnum)
    HANDLE_TYPE_COUNT
} HandleType;

typedef enum __handle_result {
    FOREACH_HANDLERESULT(GenerateEnum)
    HANDLE_RESULT_COUNT
} HandleResult;

typedef struct __handle_request {
    HandleType type;
    union {
        struct {
            const char *path;
        };
    };
} HandleRequest;

typedef struct __handle {
    HandleType type;
    bool read, write;
    uint32_t owner_pid;
    
    union {
        FsNode* fsnode;   // HANDLE_TYPE_FILE
    } content;
} Handle;

HandleResult handle_acquire(Task* task, HandleRequest request);
void handle_release(Task* task, Handle* handle);

