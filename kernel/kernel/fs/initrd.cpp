#include <kernel/fs/initrd.h>
#include <kernel/arch/i386/mm/include/heap.h>
#include <string.h>

initrd_header_t *initrd_header;
initrd_file_header_t *file_headers;
fs_node_t *initrd_root;
fs_node_t *initrd_dev;
fs_node_t *root_nodes;
int nroot_nodes;
struct dirent dirent;

static uint32_t initrd_read(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer) {
    initrd_file_header_t header = file_headers[node->inode];
    if (offset > header.length) return 0;
    if (offset + size > header.length) size = header.length - offset;
    
    // The data is located at initrd_header + header.offset
    uint8_t *data = (uint8_t *)((uint32_t)initrd_header + header.offset);
    memcpy(buffer, data + offset, size);
    return size;
}

static struct dirent *initrd_readdir(fs_node_t *node, uint32_t index) {
    if (node == initrd_root && index == 0) {
        strcpy(dirent.name, ".");
        dirent.ino = 0;
        return &dirent;
    }
    if (node == initrd_root && index == 1) {
        strcpy(dirent.name, "..");
        dirent.ino = 0;
        return &dirent;
    }
    if (index - 2 < (uint32_t)nroot_nodes) {
        strcpy(dirent.name, root_nodes[index - 2].name);
        dirent.ino = root_nodes[index - 2].inode;
        return &dirent;
    }
    return 0;
}

static fs_node_t *initrd_finddir(fs_node_t *node, const char *name) {
    if (node == initrd_root && !strcmp(name, ".")) return initrd_root;
    if (node == initrd_root && !strcmp(name, "..")) return initrd_root;
    
    for (int i = 0; i < nroot_nodes; i++) {
        if (!strcmp(name, root_nodes[i].name)) {
            return &root_nodes[i];
        }
    }
    return 0;
}

fs_node_t *initialise_initrd(uint32_t location) {
    initrd_header = (initrd_header_t *)location;
    file_headers = (initrd_file_header_t *)(location + sizeof(initrd_header_t));
    
    // Initialize the root directory
    initrd_root = (fs_node_t *)kmalloc(sizeof(fs_node_t));
    memset(initrd_root, 0, sizeof(fs_node_t));
    strcpy(initrd_root->name, "initrd");
    initrd_root->mask = 0;
    initrd_root->uid = 0;
    initrd_root->gid = 0;
    initrd_root->inode = 0;
    initrd_root->length = 0;
    initrd_root->flags = FS_DIRECTORY;
    initrd_root->read = 0;
    initrd_root->write = 0;
    initrd_root->open = 0;
    initrd_root->close = 0;
    initrd_root->readdir = &initrd_readdir;
    initrd_root->finddir = &initrd_finddir;
    initrd_root->ptr = 0;
    initrd_root->impl = 0;
    
    nroot_nodes = initrd_header->nfiles;
    root_nodes = (fs_node_t *)kmalloc(sizeof(fs_node_t) * nroot_nodes);
    
    // Create VFS nodes for each file in the initrd
    for (int i = 0; i < nroot_nodes; i++) {
        memset(&root_nodes[i], 0, sizeof(fs_node_t));
        strcpy(root_nodes[i].name, file_headers[i].name);
        root_nodes[i].mask = 0;
        root_nodes[i].uid = 0;
        root_nodes[i].gid = 0;
        root_nodes[i].length = file_headers[i].length;
        root_nodes[i].inode = i;
        root_nodes[i].flags = FS_FILE;
        root_nodes[i].read = &initrd_read;
        root_nodes[i].write = 0;
        root_nodes[i].readdir = 0;
        root_nodes[i].finddir = 0;
        root_nodes[i].open = 0;
        root_nodes[i].close = 0;
        root_nodes[i].impl = 0;
    }
    
    return initrd_root;
}
