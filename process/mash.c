/*
    A minimum shell implementation for study of processes under Linux
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

/*

*/

#define DEBUG               true
#define RUNNING             true
// Allow 64 strings in path initially, add 16 each time if needed
#define PATH_INITIAL_SIZE   64
#define PATH_INCREMENT_SIZE 16

int32_t init_path(char** path, int32_t* count);
int32_t inspect(const char* var);
// int32_t add_path(const char* newPath);
int32_t remove_path(const char* oldPath, char** path, int32_t pathCount);

int main(int argc, char* argv[]) {
    char** path = malloc(PATH_INITIAL_SIZE * sizeof(char*));
    int32_t pathCount = 0;
    if (init_path(path, &pathCount) == EXIT_FAILURE) {
        fprintf(stderr, "init_path() failed\n");
        return EXIT_FAILURE;
    }
    printf("init_path() successfully completed, %d paths loaded\n", pathCount);

    while(RUNNING) {
        fprintf(stdout, ">");
        char* input = NULL;
        size_t size;
        if (getline(&input, &size, stdin) == -1) {
            fprintf(stderr, "getline() error\n");
        }
        printf(">%s\n", input);
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

int32_t init_path(char** path, int32_t* count) {
    /*

    */

    // count must be NULL as we only initiate path at the beginning
    if (*count > 0) {
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
            (*count)++;
        }
        if (s[i] == '\0') {
            (*count)++;
            break;
        }
    }
    printf("There are a total of %d paths\n", *count);

    // Start parsing
    int32_t start = 0;
    int32_t end = 0;
    int32_t pathCount = 0;
    
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
            path[pathCount] = p;
            printf("%s\n", p);
            pathCount++;
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

int32_t inspect(const char* var) {
    /*

    */
    return EXIT_SUCCESS;
}

int32_t remove_path(const char* oldPath, char** path, int32_t pathCount) {
    /*
        Seach for oldPath in path;
        Once found at position i then remove it
    */
    for (int i = 0; i < pathCount; i++) {
        
    }

}