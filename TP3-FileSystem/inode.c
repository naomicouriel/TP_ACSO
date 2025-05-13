#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include "inode.h"
#include "diskimg.h"

#include <string.h>
#include "unixfilesystem.h"
#include <stdint.h>

#define INODES_PER_BLOCK (DISKIMG_SECTOR_SIZE / sizeof(struct inode))
#define PTRS_PER_BLOCK (DISKIMG_SECTOR_SIZE / sizeof(uint16_t))

/**
 * TODO
 */
int inode_iget(struct unixfilesystem *fs, int inumber, struct inode *inp) {
    int total_inodes = fs->superblock.s_isize * INODES_PER_BLOCK;
    if (inumber < 1 || inumber > total_inodes) {
        fprintf(stderr, "Error: inode_iget: Invalid inode number %d (max: %d)\n", inumber, total_inodes);
        return -1;
    }

    int zero_based_inumber = inumber - 1;
    int inode_block_num = INODE_START_SECTOR + (zero_based_inumber / INODES_PER_BLOCK);
    int offset_in_block = (zero_based_inumber % INODES_PER_BLOCK) * sizeof(struct inode);

    char block_buffer[DISKIMG_SECTOR_SIZE];
    int bytes_read = diskimg_readsector(fs->dfd, inode_block_num, block_buffer);

    if (bytes_read != DISKIMG_SECTOR_SIZE) {
        fprintf(stderr, "Error: inode_iget: Failed to read block %d for inode %d\n", inode_block_num, inumber);
        return -1;
    }

    memcpy(inp, block_buffer + offset_in_block, sizeof(struct inode));

    return 0;
}

/**
 * TODO
 */
int inode_indexlookup(struct unixfilesystem *fs, struct inode *inp,
    int blockNum) {  
    int file_size = inode_getsize(inp);

    if (file_size <= 0) {
            return -1;
    }

    int max_block_num = (file_size - 1) / DISKIMG_SECTOR_SIZE;
    if (blockNum < 0 || blockNum > max_block_num) {
        return -1;
    }

    uint16_t indirect_block[PTRS_PER_BLOCK];

    if ((inp->i_mode & ILARG) == 0) {
        if (blockNum < 8) {
            return inp->i_addr[blockNum];
        } else {
            return -1;
        }
    } else {
        int blocks_in_single = 7 * PTRS_PER_BLOCK;

        if (blockNum < blocks_in_single) {
            int indirect_block_index = blockNum / PTRS_PER_BLOCK;
            int index_in_indirect = blockNum % PTRS_PER_BLOCK;

            uint16_t singly_indirect_sector = inp->i_addr[indirect_block_index];
            if (singly_indirect_sector == 0) {
                return -1;
            }

            int bytes_read = diskimg_readsector(fs->dfd, singly_indirect_sector, indirect_block);
            if (bytes_read != DISKIMG_SECTOR_SIZE) {
                return -1;
            }

            return indirect_block[index_in_indirect];

        } else {
            int blockNum_relative = blockNum - blocks_in_single;
            int double_index = blockNum_relative / PTRS_PER_BLOCK;
            int single_index = blockNum_relative % PTRS_PER_BLOCK;

            if (double_index >= (int)PTRS_PER_BLOCK) {
                    return -1;
            }

            uint16_t doubly_indirect_sector = inp->i_addr[7];
                if (doubly_indirect_sector == 0) {
                    return -1;
                }

            uint16_t doubly_indirect_block_content[PTRS_PER_BLOCK];
            int bytes_read1 = diskimg_readsector(fs->dfd, doubly_indirect_sector, doubly_indirect_block_content);
            if (bytes_read1 != DISKIMG_SECTOR_SIZE) {
                return -1;
            }

            uint16_t singly_indirect_sector = doubly_indirect_block_content[double_index];
                if (singly_indirect_sector == 0) {
                    return -1;
                }

            int bytes_read2 = diskimg_readsector(fs->dfd, singly_indirect_sector, indirect_block);
                if (bytes_read2 != DISKIMG_SECTOR_SIZE) {
                return -1;
            }

            return indirect_block[single_index];
        }
    }
}

int inode_getsize(struct inode *inp) {
  return ((inp->i_size0 << 16) | inp->i_size1); 
}
