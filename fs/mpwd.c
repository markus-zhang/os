#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>

ino_t get_inode(char* fname);
void inode_to_name(ino_t inode, char* namebuf, int buflen);
void run(ino_t current_dir_inode);

int main() {
    ino_t current_dir_inode = get_inode(".");
    run(current_dir_inode);
    printf("\n");
    return 0;
}

/**
 * @brief recursively find parent directory until reaches top,
 * and then print the names of the directory starting from root
 * 
 * @param current_dir_inode 
 */
void run(ino_t current_dir_inode) {
    ino_t parent_dir_inode = get_inode("..");
    if (current_dir_inode == parent_dir_inode) {
        // breaking the recursive algorithm
        // print top directory
        printf("/");
        return;
    }
    else {
        /**
         * @brief Remember that we have to step into parent directory
         * to grab the "real" name of current directory.
         * And becuase we want to display the names in sort of backward order,
         * considering we will reach top directory last, but print its name first,
         * we need to do two things correctly:
         * 1) We need to grab the real name of current directory BEFORE next recursive run,
         * because next run will chdir again;
         * 2) We need to push back printf(current_dir) AFTER next recursive run,
         * because we want to display in backward order
         * 
         */

        // If you wonder why chdir() does not seem to change the cwd
        // when the program ends and control returns to shell,
        // recall that each process has its own cwd
        chdir("..");
        char current_dir[1024];
        inode_to_name(current_dir_inode, current_dir, 1024);
        run(parent_dir_inode);        
        printf("%s/", current_dir);
    }
}

void inode_to_name(ino_t inode, char* namebuf, int buflen) {
    DIR* ptr_dir;
    struct dirent* ptr_dirent;
    ptr_dir = opendir(".");
    if (ptr_dir == NULL) {
        perror(".");
        exit(1);
    }

    /**
     * @brief readdir() walks through all files under given directory,
     * including . and .., the current and parent directory.
     * Becuase it doesn't always hit . or .. first, we need to compare
     * the inode of each file with the inode of . and ..
     * 
     */

    while ((ptr_dirent = readdir(ptr_dir)) != NULL) {
        if (ptr_dirent->d_ino == inode) {
            strncpy(namebuf, ptr_dirent->d_name, buflen);
            // Technically strncpy should copy the ending '\0'
            namebuf[buflen - 1] = '\0';
            closedir(ptr_dir);
            return;
        }
    }
    fprintf(stderr, "error looking for inode %ld\n", inode);
    exit(1);
}

ino_t get_inode(char* fname) {
    /**
     * @brief returns inode number of the file
     * 
     */

    struct stat info;
    if (stat(fname, &info) == -1) {
        fprintf(stderr, "Cannot stat ");
        perror(fname);
        exit(1);
    }
    return info.st_ino;
}