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

int main(int argc, char** argv) {
    if (argc >= 4 || argc <= 1) {
        usage();
        exit(1);
    }

    DIR *dirp;
    struct dirent* dp;

    if (argc == 2) {
        // default to cwd
        // chdir(".");
        if ((dirp = opendir(".")) == NULL) {
            perror(argv[1]);
        }
    }
    else {
        // if (chdir(argv[1]) == -1) {
        //     perror(argv[1]);
        //     exit(1);
        // }
        if ((dirp = opendir(argv[2])) == NULL) {
            perror(argv[2]);
        }
    }
    
    while((dp = readdir(dirp))) {
        printf("%s ", dp->d_name);
    }
    putchar('\n');
    return 0;
}

void usage() {
    // print usage
    printf("Usage: ./find <file name> [directory name]\n");
}