/*
    A minimum shell implementation for study of processes under Linux
    gcc -std=gnu99 -Wall -Werror -pedantic mash.c -o mash
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define __STDC_WANT_LIB_EXT1__ 1
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
// #define _DEFAULT_SOURCE
#include <dirent.h>
#include "mashtool.h"

/*
    TODO: Summarize the functionalities of MASH
*/

#define DEBUG                   true
#define RUNNING                 true
// Allow 64 strings in path initially, add 16 each time if needed
#define PATH_INITIAL_SIZE       64
#define PATH_INCREMENT_SIZE     16
#define CWD_LEN                 256
#define ARGV_INITIAL_SIZE       64
#define ARGV_INCREMENT_SIZE     16

int32_t init_path();
int32_t inspect_path();
int32_t add_path(const char* newPath);
int32_t remove_path(const char* oldPath);
int32_t process_command(char* command);
int32_t _internal_ls();

/*
    Global variables
    - cwd: current working directory
        - type: C string
    - path: collection of all directories in $PATH
        - type: array of C strings
    - pathCount: count of paths in $PATH
        - type: int32_t
        - comment: changes for each remove/add operation
*/
char*       cwd;
char**      path;
int32_t     pathCount;
char*       argv[ARGV_INITIAL_SIZE];

int main(int argc, char* argv[]) {
    // Initiate CWD
    cwd = malloc(CWD_LEN);
    if (getcwd(cwd, CWD_LEN) == NULL) {
        fprintf(stderr, "getcwd() failed\n");
        return EXIT_FAILURE;
    }
    printf("CWD: %s\n", cwd);
    
    // Initiate PATH
    path = malloc(PATH_INITIAL_SIZE * sizeof(char*));
    pathCount = 0;
    if (init_path() == EXIT_FAILURE) {
        fprintf(stderr, "init_path() failed\n");
        return EXIT_FAILURE;
    }

    // Initiate ARGV
    for (int i = 0; i < ARGV_INITIAL_SIZE; i++) {
        argv[i] = NULL;
    }

    while(RUNNING) {
        fprintf(stdout, ">");
        char* input = NULL;
        size_t size;
        if (getline(&input, &size, stdin) == -1) {
            fprintf(stderr, "getline() error\n");
        }
        /*
            Process user input
            - built-in command:
                - ls: list all files and directories under cwd
        */
        process_command(input);
    }

    // shut down
    for (int i = 0; i < pathCount; i++) {
        free(path[pathCount]);
        path[pathCount] = NULL;
        free(path);
        path = NULL;
    }
    exit(EXIT_SUCCESS);
}

int32_t init_path() {
    /*
        TODO: Summarize the functionality of this function
    */

    // count must be NULL as we only initiate path at the beginning
    if (pathCount > 0) {
        fprintf(stderr, "init_path(): path count must be 0\n");
        return EXIT_FAILURE;
    }
    // getenv() returns a pointer to a string within the environment list.
    // The caller must take care not to modify this string
    const char* s = getenv("PATH");
    if (s == NULL) {
        fprintf(stderr, "init_path(): getenv() failed\n");
    }
    // Set count
    for (int i = 0;;i++) {
        if (s[i] == ':') {
            pathCount++;
        }
        if (s[i] == '\0') {
            pathCount++;
            break;
        }
    }
    printf("There are a total of %d paths\n", pathCount);

    // Start parsing
    int32_t start = 0;
    int32_t end = 0;
    int32_t count = 0;
    
    while (true) {
        if (s[end] == ':' || s[end] == '\0') {
            // split s[start:end] into a new string
            int32_t length = end - start;
            // consider \0 at end
            char* p = malloc(length + 1);
            for (int i = 0; i < length; i++) {
                p[i] = s[start + i];
            }
            p[length] = '\0';
            path[count] = p;
            // TODO: convert next line to debugging
            // printf("%s\n", p);
            count++;
            if (s[end] == '\0') {
                break;
            }
            else {
                end++;
                // skip :
                start = end;
            }
        }
        else {
            end++;
        }
    }
    return EXIT_SUCCESS;
}

int32_t inspect_path() {
    /*
        TODO: Summarize the functionality of this function
    */
    printf("There are a total of %d paths\n", pathCount);
    for (int i = 0; i < pathCount; i++) {
        printf("%s\n", path[i]);
    }
    
    return EXIT_SUCCESS;
}

int32_t add_path(const char* newPath) {
    /*
        If more than 64 already then refuses to add
        TODO: re allocate 64+16 and copy to new array
    */
    if (pathCount >= PATH_INCREMENT_SIZE) {
        return EXIT_FAILURE;
    }
    size_t len = strlen(newPath);
    char* temp = malloc(len + 1);
    for (int i = 0; i < len; i++) {
        temp[i] = newPath[i];
    }
    temp[len] = '\0';
    path[pathCount] = temp;
    pathCount++;
    return EXIT_SUCCESS;
}

int32_t remove_path(const char* oldPath) {
    /*
        Seach for oldPath in path;
        Once found at position i then remove it
    */
    for (int i = 0; i < pathCount; i++) {
        if (strcmp(path[i], oldPath) == 0) {
            // remove by moving pointers after
            for (int j = i; j < pathCount - 1; j++) {
                path[j] = path[j + 1];
            }
            path[pathCount - 1] = NULL;
            (pathCount)--;
            return EXIT_SUCCESS;
        }
    }
    return EXIT_FAILURE;
}

int32_t process_command(char* command) {
    if (strlen(command) == 0) {
        return EXIT_SUCCESS;
    }

    // TODO: Tokenize command and put into argv[]
    // TODO: Check if <= 64 tokens, if > then still parse but warn about lost data
    if (count_char(command, ' '))
    
    if ((strncmp(command, "ls\n", 3) == 0) && (strlen(command) == 3)) {
        return _internal_ls();
    }
    
    return EXIT_SUCCESS;
}

int32_t _internal_ls() {
    /*
        TODO: Summarize the functionality of this function
        https://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program
    */
    
    // TODO: implement some flags for ls
    DIR* d;
    struct dirent* dir;
    d = opendir(".");
    int32_t fileCount = 0;
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_type == DT_REG) {
                fileCount++;
            }
        }
        // Now load all filenames into an array
        // First rewind to beginning of folder
        rewinddir(d);
        char** filename = malloc(fileCount * sizeof(char*));
        fileCount = 0;
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_type == DT_REG) {
                int len = strlen(dir->d_name) + 1;
                char* n = malloc(len + 1);
                strncpy(n, dir->d_name, len);

                filename[fileCount] = n;
                fileCount++;
            }
        }
        // Insertion sort, use strcasecmp() to compare
        // First compare 1 with 0, obtain a stable 0~1
        // Next compare 2 with 0~1, obtain a stable 0~2
        // ...until compare n-1 with 0~n-2, obtain a stable 0~n-1, done
        int32_t next = 1;
        while (next < fileCount) {
            int32_t walker = next;
            while (walker > 0 && strcasecmp(filename[walker - 1], filename[walker]) > 0) {
                char* temp = filename[walker];
                filename[walker] = filename[walker - 1];
                filename[walker - 1] = temp;
                walker--;
            }
            next++;
        }
        // test print
        for (int i = 0; i < fileCount; i++) {
            printf("%s  ", filename[i]);
        }
        printf("\n");
        // Clean up
        for (int i = 0; i < fileCount; i++) {
            free(filename[i]);
        }
        free(filename);
        closedir(d);
    }
    return EXIT_SUCCESS;
}