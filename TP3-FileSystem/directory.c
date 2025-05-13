#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include "file.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <string.h>
#include "unixfilesystem.h"

#define DIRENTV6_SIZE sizeof(struct direntv6)

/**
 * TODO
 */
int directory_findname(struct unixfilesystem *fs, const char *name, int dirinumber, struct direntv6 *dirEnt) {
  struct inode dir_inode;

  if (inode_iget(fs, dirinumber, &dir_inode) < 0) {
      return -1;
  }

  if (! (dir_inode.i_mode & IALLOC)) {
        return -1;
  }
  if ((dir_inode.i_mode & IFMT) != IFDIR) {
      return -1;
  }

  int dir_size = inode_getsize(&dir_inode);

  if (dir_size <= 0) {
        return -1;
  }
  if (dir_size % DIRENTV6_SIZE != 0) {
      return -1;
  }

  char block_buffer[DISKIMG_SECTOR_SIZE];

  int num_blocks = (dir_size + DISKIMG_SECTOR_SIZE - 1) / DISKIMG_SECTOR_SIZE;

  for (int blockNum = 0; blockNum < num_blocks; blockNum++) {
      int valid_bytes = file_getblock(fs, dirinumber, blockNum, block_buffer);
      if (valid_bytes < 0) {
          return -1;
      }
        if (valid_bytes == 0) continue;


      int num_entries_in_block = valid_bytes / DIRENTV6_SIZE;

      for (int i = 0; i < num_entries_in_block; i++) {
          struct direntv6 *current_entry = (struct direntv6 *)(block_buffer + i * DIRENTV6_SIZE);

          if (current_entry->d_inumber == 0) {
              continue;
          }

          if (strncmp(name, current_entry->d_name, sizeof(current_entry->d_name)) == 0) {
              memcpy(dirEnt, current_entry, DIRENTV6_SIZE);
              return 0;
          }
      }
  }

  return -1;
}
