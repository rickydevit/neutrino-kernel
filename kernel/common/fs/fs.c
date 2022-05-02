#include "fs.h"
#include <_null.h>

FsNode* root = nullptr;

// === PRIVATE FUNCTIONS ========================


// === PUBLIC FUNCTIONS =========================

uintptr_t fs_read(FsNode* node, uint32_t offset, uint32_t size, uint8_t* buf) {
    if (node->read != nullptr)
        return node->read(node, offset, size, buf);
    else
        return nullptr;
}

uintptr_t fs_write(FsNode* node, uint32_t offset, uint32_t size, uint8_t* buf) {
    if (node->write != nullptr)
        return node->write(node, offset, size, buf);
    else
        return nullptr;
}

void fs_open(FsNode* node, uint8_t read, uint8_t write) {
    if (node->open != nullptr)
        return node->open(node);
    else
        return nullptr;
}

void fs_close(FsNode* node) {
    if (node->close != nullptr)
        return node->close(node);
    else
        return nullptr;
}

struct __dirent* fs_readdir(FsNode* node, uint32_t index) {
    if (IsDirectory(node) && node->readdir != nullptr)
        return node->readdir(node, index);
    else
        return nullptr;
}

FsNode* fs_finddir(FsNode* node, char* name) {
    if (IsDirectory(node) && node->finddir != nullptr)
        return node->finddir(node, name);
    else
        return nullptr;
}
