#ifndef _KERNEL_FS_INITRD_H
#define _KERNEL_FS_INITRD_H

#include <stdint.h>
#include <kernel/fs/vfs.h>

typedef struct {
    uint32_t nfiles;
} initrd_header_t;

typedef struct {
    char name[64];
    uint32_t offset;
    uint32_t length;
} initrd_file_header_t;

/**
 * @brief Initializes the initial ramdisk.
 * @param location Physical address of the loaded initrd module.
 * @return The root VFS node for the initrd.
 */
fs_node_t *initialise_initrd(uint32_t location);

#endif
