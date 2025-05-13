
#include "pathname.h"
#include "directory.h"
#include "inode.h"
#include "diskimg.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include <stdlib.h>
#include <string.h>
#include "unixfilesystem.h"

/**
 * TODO
 */
int pathname_lookup(struct unixfilesystem *fs, const char *pathname) {
    if (!pathname || pathname[0] != '/') {
        return -1;
    }
    if (strcmp(pathname, "/") == 0) {
        return ROOT_INUMBER;
    }

    char *path_copy = strdup(pathname);
    if (!path_copy) {
        perror("Error duplicating pathname");
        return -1;
    }

    int current_inumber = ROOT_INUMBER;
    struct direntv6 found_entry;
    struct inode component_inode;

    char *component = strtok(path_copy + 1, "/");

    while (component != NULL) {
        if (strlen(component) >= sizeof(found_entry.d_name)) {
             free(path_copy);
             return -1;
        }

        if (directory_findname(fs, component, current_inumber, &found_entry) < 0) {
            free(path_copy);
            return -1;
        }

        int next_inumber = found_entry.d_inumber;

        char *peek_next = strtok(NULL, "/");

        if (peek_next != NULL) {
            if (inode_iget(fs, next_inumber, &component_inode) < 0) {
                 free(path_copy);
                 return -1;
            }
            if ((component_inode.i_mode & IFMT) != IFDIR) {
                free(path_copy);
                return -1;
            }
        }

        current_inumber = next_inumber;
        component = peek_next;
    }

    free(path_copy);

    return current_inumber;
}