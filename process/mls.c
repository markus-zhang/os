#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define __STDC_WANT_LIB_EXT1__ 1
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
// #define _DEFAULT_SOURCE
#include <dirent.h>
#include <sys/stat.h>
#include "mashtool.h"

typedef enum filetype
{
    symbolic_link,
    named_pipe,
    directory,
    arg_directory,          /* Directory given as command line arg. Ended with forward slash '/' */
    normal			        /* All others. */
} filetype;

typedef struct file {
    /* The file name */
    char* name;

    struct stat stat;

    /* For symbolic link, name of the file linked to, otherwise NULL */
    char* link_name;

    filetype filetype;

} file;

int main(int argc, char* argv[]) {
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