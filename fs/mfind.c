/**
 * @file find.c
 * @author Markus Zhang (markus_zhang@hotmail.com)
 * @brief A command line tool that finds a file in the designated folder.
 * Output: displays the folder if found, or Non Inventum if not
 * @version 0.1
 * @date 2023-05-14
 * 
 * @copyright Copyleft (c) 2023
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

void usage();
void recursive_find(char* filename);
void printcwd();

int main(int argc, char** argv) {
    if (argc >= 4 || argc <= 1) {
        usage();
        exit(1);
    }

    // DIR *dirp;
    // struct dirent* dp;

    // if (argc == 2) {
    //     // default to cwd
    //     // chdir(".");
    //     if ((dirp = opendir(".")) == NULL) {
    //         perror(argv[1]);
    //     }
    // }
    // else {
    //     // if (chdir(argv[1]) == -1) {
    //     //     perror(argv[1]);
    //     //     exit(1);
    //     // }
    //     if ((dirp = opendir(argv[2])) == NULL) {
    //         perror(argv[2]);
    //     }
    // }

    if (argc == 2) {
        // default to cwd
        chdir(".");
        // if ((dirp = opendir(".")) == NULL) {
        //     perror(argv[1]);
        //     exit(1);
        // }
    }
    else {
        if (chdir(argv[1]) == -1) {
            perror(argv[1]);
            exit(1);
        }
    }
    recursive_find(argv[1]);
    
    /**
     * @brief For each directory entry, check if there is a match
     * Then check if the entry is a directory, if so, 
     * recursively search in entry
     * 
     */
    // while((dp = readdir(dirp))) {
    //     int namelen = strlen(dp->d_name) + 1;
    //     if (strncmp(dp->d_name, argv[1], namelen) == 0) {
    //         // Found!
    //         printf("Found at: %s\n", argv[2]);
    //     }
    //     printf("%s ", dp->d_name);
    // }
    putchar('\n');
    return 0;
}

void recursive_find(char* filename) {
    // printcwd();
    if (filename == NULL) {
        fprintf(stderr, "Empty filename\n");
        exit(1);
    }
    DIR *dirp;
    struct dirent* dp;

    if ((dirp = opendir(".")) == NULL) {
        perror("cannot open cwd\n");
        exit(1);
    }

    while((dp = readdir(dirp))) {
        int namelen = strlen(dp->d_name) + 1;
        if (strncmp(dp->d_name, filename, namelen - 1) == 0) {
            // Found!
            // printf("%s\n", dp->d_name);
            char dir[1024];
            if (getcwd(dir, 1024) == NULL) {
                fprintf(stderr, "getcwd error\n");
                exit(1);
            }
            printf("Found at: %s\n", dir);
        }
        if (dp->d_type == DT_DIR && dp->d_name[0] != '.') {
            // A directory! Let's check inside...
            // printf("Ready to switch to: %s\n", dp->d_name);
            chdir(dp->d_name);
            recursive_find(filename);
        }
    }
    // Need to go back to parent in case we chdir() into child dir
    // Even if we never do that, it doesn't hurt because top dir is it's own parent
    chdir("..");
}

void printcwd() {
    char dir[1024];
    if (getcwd(dir, 1024) == NULL) {
        fprintf(stderr, "getcwd error\n");
        exit(1);
    }
    printf("CWD: %s\n", dir);
}

void usage() {
    // print usage
    printf("Usage: ./find <file name> [directory name]\n");
}