#include "handle.h"
#include <linkedlist.h>
#include <neutrino/lock.h>
#include <liballoc.h>
#include <string.h>

#include "../fs/fs.h"
#include "../tasks/task.h"

List* global_handles = nullptr;
Lock handles_lock = NewLock;

// === PRIVATE FUNCTIONS ========================

Handle* handle_get_or_create_fsnode(const char* path) {
    int i = 0;
    struct __dirent* node = 0;

    while ((node = fs_readdir(root, i)) != nullptr) {
        FsNode* fsnode = fs_finddir(root, node->name);

        if (IsFile(fsnode) && strcmp(fsnode->name, path)) {
            List* l = list_find_node_with(global_handles, (void*)node);

            if (l != nullptr) {
                return (Handle*)list_get_value(l);
            } else {
                Handle* d = (Handle*)kmalloc(sizeof(Handle));
                d->content.fsnode = fsnode;
                d->read = true;
                d->write = false;       // TODO: change this
                
                global_handles = list_append(global_handles, (void*)d);
                return d;
            }
        }
        i++;
    }

    return nullptr;
}

// === PUBLIC FUNCTIONS =========================

HandleResult handle_acquire(Task* task, HandleRequest request) {
    if (task == nullptr || request.type >= HANDLE_TYPE_COUNT) return HANDLE_RESULT_BAD_REQUEST;

    LockRetain(handles_lock);
    Handle* handle = nullptr;

    switch (request.type) {
        case HANDLE_TYPE_FILE:
            if (request.path == nullptr) return HANDLE_RESULT_BAD_REQUEST;
            handle = handle_get_or_create_fsnode(request.path);
            break;
        
        default: return HANDLE_RESULT_BAD_REQUEST;
    }

    if (handle != nullptr) {
        if (handle->owner_pid != 0) return HANDLE_RESULT_ALREADY_OWNED;    // descriptor is already owned
        else handle->owner_pid = task->pid;

        task->handles = list_append(task->handles, handle);
        return HANDLE_RESULT_SUCCESS;
    }

    return HANDLE_RESULT_ERROR;
}

void handle_release(Task* task, Handle* handle) {
    if (task == nullptr || handle == nullptr) return;
    
    LockRetain(handles_lock);
    List* p = global_handles;
    size_t i = 0;

    while (p != nullptr) {
        Handle* data = list_get_value(p);
        
        if (data == handle) {
            global_handles = list_delete_at(global_handles, i);
            return;
        }

        p = p->next;
        i++;
    }
}
