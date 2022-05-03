#include "initrd.h"
#include "fs.h"
#include "../memory/memory.h"
#include <_null.h>
#include <stdint.h>
#include <string.h>
#include <liballoc.h>

InitrdHeader* initrd_header;
InitrdFileHeader* file_headers;
FsNode* initrd_root;
FsNode* initrd_dev;
FsNode* root_nodes;
size_t nroot_nodes;

struct __dirent dirent;

// === PRIVATE FUNCTIONS ========================

static size_t initrd_read(FsNode* node, uint32_t offset, uint32_t size, uint8_t* buffer) {
    InitrdFileHeader header = file_headers[node->inode];
    
    if (offset > header.length) return 0;
    if (offset + size > header.length) size = header.length - offset; 

    memory_copy((uint8_t*)(uintptr_t)(header.offset+offset), buffer, size);
    return size;
}

static struct __dirent* initrd_readdir(FsNode* node, uint32_t index) {
    if (node == initrd_root && index == 0) {
        strcpy("dev", dirent.name);
        dirent.name[3] = 0;
        dirent.ino = 0;
        return &dirent;
    }

    if (index-1 >= nroot_nodes) return nullptr;

    strcpy(root_nodes[index-1].name, dirent.name);
    dirent.name[strlen(root_nodes[index-1].name)] = 0;
    dirent.ino = root_nodes[index-1].inode;
    return &dirent;
}

static FsNode* initrd_finddir(FsNode* node, char* name) {
    if (node == initrd_root && !strcmp(name, "dev"))
        return initrd_dev;
    
    for (size_t i = 0; i < nroot_nodes; i++)
        if (!strcmp(name, root_nodes[i].name))
            return &root_nodes[i];   

    return nullptr;
}

// === PUBLIC FUNCTIONS =========================

FsNode* init_initrd(uintptr_t addr) {
    initrd_header = (InitrdHeader*)addr;
    file_headers = (InitrdFileHeader*)(addr + sizeof(InitrdHeader));

    // root dir init
    {
        initrd_root = (FsNode*) kmalloc(sizeof(FsNode));
        strcpy("initrd", initrd_root->name);
        initrd_root->perm_mask = initrd_root->user_id = initrd_root->group_id = initrd_root->inode = initrd_root->length = 0;
        initrd_root->flags = FS_DIRECTORY;
        initrd_root->read = nullptr;
        initrd_root->write = nullptr;
        initrd_root->open = nullptr;
        initrd_root->close = nullptr;
        initrd_root->readdir = &initrd_readdir;
        initrd_root->finddir = &initrd_finddir;
        initrd_root->ptr = nullptr;
        initrd_root->impl = NULL;
    }

    // dev dir init
    {
        initrd_dev = (FsNode*) kmalloc(sizeof(FsNode));
        strcpy("dev", initrd_dev->name);
        initrd_dev->perm_mask = initrd_dev->user_id = initrd_dev->group_id = initrd_dev->inode = initrd_dev->length = 0;
        initrd_dev->flags = FS_DIRECTORY;
        initrd_dev->read = nullptr;
        initrd_dev->write = nullptr;
        initrd_dev->open = nullptr;
        initrd_dev->close = nullptr;
        initrd_dev->readdir = &initrd_readdir;
        initrd_dev->finddir = &initrd_finddir;
        initrd_dev->ptr = nullptr;
        initrd_dev->impl = NULL;
    }
    root_nodes = (FsNode*) kmalloc(sizeof(FsNode)*initrd_header->n_files);
    nroot_nodes = (size_t)initrd_header->n_files;

    // files
    for (int i = 0; i < initrd_header->n_files; i++) {
        file_headers[i].offset += addr;
        strcpy(file_headers[i].name, root_nodes[i].name);
        root_nodes[i].perm_mask = root_nodes[i].user_id = root_nodes[i].group_id = 0;
        root_nodes[i].length = file_headers[i].length;
        root_nodes[i].inode = i;
        root_nodes[i].flags = FS_FILE;
        root_nodes[i].read = &initrd_read;
        root_nodes[i].write = nullptr;
        root_nodes[i].readdir = nullptr;
        root_nodes[i].finddir = nullptr;
        root_nodes[i].open = nullptr;
        root_nodes[i].close = nullptr;
        root_nodes[i].impl = NULL;
    }

    return initrd_root;
}
