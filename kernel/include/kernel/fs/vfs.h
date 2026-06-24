/**
 * @file vfs.h
 * @brief Virtual File System (VFS) abstraction layer structures and definitions.
 *
 * Defines the core structures for the VFS, including standard file operations,
 * directory entries, and the generic filesystem node.
 */

#ifndef _KERNEL_FS_VFS_H
#define _KERNEL_FS_VFS_H

#include <stdint.h>

#define FS_FILE        0x01
#define FS_DIRECTORY   0x02
#define FS_CHARDEVICE  0x03
#define FS_BLOCKDEVICE 0x04
#define FS_PIPE        0x05
#define FS_SYMLINK     0x06
#define FS_MOUNTPOINT  0x08

struct fs_node;

typedef uint32_t (*read_type_t)(struct fs_node*, uint32_t, uint32_t, uint8_t*);
typedef uint32_t (*write_type_t)(struct fs_node*, uint32_t, uint32_t, uint8_t*);
typedef void (*open_type_t)(struct fs_node*);
typedef void (*close_type_t)(struct fs_node*);
typedef struct dirent* (*readdir_type_t)(struct fs_node*, uint32_t);
typedef struct fs_node* (*finddir_type_t)(struct fs_node*, const char* name);

/**
 * @brief Represents an entry in a directory.
 */
struct dirent {
    char name[128]; // Filename
    uint32_t ino;   // Inode number
};

/**
 * @brief Represents a generic filesystem node (file, directory, device, etc).
 */
struct fs_node {
    char name[128];     // The requested name
    uint32_t mask;      // Permissions mask
    uint32_t uid;       // Owning user
    uint32_t gid;       // Owning group
    uint32_t flags;     // Node type
    uint32_t inode;     // Device-specific inode number
    uint32_t length;    // Size of the file in bytes
    uint32_t impl;      // Implementation-defined number
    
    read_type_t read;
    write_type_t write;
    open_type_t open;
    close_type_t close;
    readdir_type_t readdir;
    finddir_type_t finddir;
    
    struct fs_node* ptr; // Used by mountpoints and symlinks
};

typedef struct fs_node fs_node_t;

extern fs_node_t *fs_root; // The root of the filesystem

uint32_t vfs_read(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
uint32_t vfs_write(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer);
void vfs_open(fs_node_t *node, uint8_t read, uint8_t write);
void vfs_close(fs_node_t *node);
struct dirent *vfs_readdir(fs_node_t *node, uint32_t index);
fs_node_t *vfs_finddir(fs_node_t *node, const char *name);

#endif // _KERNEL_FS_VFS_H
