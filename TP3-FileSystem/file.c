#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "file.h"
#include "inode.h"
#include "diskimg.h"

#include <string.h>
#include "unixfilesystem.h"

/**
 * TODO
 */
int file_getblock(struct unixfilesystem *fs, int inumber, int blockNum, void *buf) {
    struct inode file_inode;
    int bytes_read;
    int valid_bytes = 0;

    if (inode_iget(fs, inumber, &file_inode) < 0) {
        return -1;
    }

    if ((file_inode.i_mode & IALLOC) == 0) {
        return -1;
    }

    int physical_block_num = inode_indexlookup(fs, &file_inode, blockNum);

    if (physical_block_num < 0) {
        return -1;
    }

    int file_size = inode_getsize(&file_inode);
    if (file_size <= 0) {
        return 0;
    }

    if (physical_block_num == 0) {
        memset(buf, 0, DISKIMG_SECTOR_SIZE);
    } else {
        bytes_read = diskimg_readsector(fs->dfd, physical_block_num, buf);
        if (bytes_read != DISKIMG_SECTOR_SIZE) {
            return -1;
        }
    }

    int last_block_index = (file_size - 1) / DISKIMG_SECTOR_SIZE;

    if (blockNum < last_block_index) {
        valid_bytes = DISKIMG_SECTOR_SIZE;
    } else if (blockNum == last_block_index) {
        int remainder = file_size % DISKIMG_SECTOR_SIZE;
        if (remainder == 0) {
            valid_bytes = DISKIMG_SECTOR_SIZE;
        } else {
            valid_bytes = remainder;
        }
    } else {
        valid_bytes = 0;
    }

    return valid_bytes;
}