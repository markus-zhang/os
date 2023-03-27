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
#include <getopt.h>
#include "mashtool.h"

#define COLOR_RED   "/033[0;31m]"

int32_t decode_switches(int argc, char* argv);
void usage();
void clear_files();
int gobble_file(char* name);

enum filetype {
    symbolic_link,
    named_pipe,
    directory,
    arg_directory,          /* Directory given as command line arg. Ended with forward slash '/' */
    normal			        /* All others. */
};

struct file {
    /* The file name */
    char* name;

    struct stat stat;

    /* For symbolic link, name of the file linked to, otherwise NULL */
    char* link_name;

    enum filetype filetype;

};

/* Do we trace links to pointed object or not? */

bool trace_links;

/* Array of files passed in argv */

struct file* files;

/* Length of block that `files' points to, measured in files. */
// This determines the allocated space of files//

int nfiles;

/* Index of first unused in `files'. */

int files_index;

/* Record of one pending directory waiting to be listed.  */

struct pending {
  char *name;
  /* If the directory is actually the file pointed to by a symbolic link we
     were told to list, `realname' will contain the name of the symbolic
     link, otherwise zero. */
  char *realname;
  struct pending *next;
};

/* Array of pending directories */

struct pending *pending_dirs;

/* sort type to be used in different sorting options */

enum sort_type {
  sort_none,			/* -U */
  sort_name,			/* default */
  sort_extension,		/* -X */
  sort_time,			/* -t */
  sort_size			    /* -S */
};

enum sort_type sort_type;
bool sort_reverse;


enum format {
    short_format,       /* default */
    long_format,		/* -l */
    one_per_line,		/* -1 */
    with_commas			/* -m */
};

enum format format;


/* none means don't mention the type of files.
   all means mention the types of all files.
   not_programs means do so except for executables.
   Controlled by -F and -p.  */

enum indicator_style {
  none,				    /* default */
  all				    /* -F */
};

enum indicator_style indicator_style;


/* Nonzero means don't omit files whose names start with `.'.  -A */

bool all_files;

/* Nonzero means don't omit files `.' and `..'
   This flag implies `all_files'.  -a  */

bool really_all_files;

/* 
    Nonzero means we are listing the working directory because no
    non-option arguments were given. 
    Example: ls -aml
*/

bool dir_defaulted;

/* Entry point of the program */

int 
main(int argc, char* argv[]) {
    int32_t i;

    /* 
        Initialize global variables;
        Put all switches to default value;
    */

    files = NULL;
    pending_dirs = NULL;
    sort_type = sort_name;
    sort_reverse = false;
    format = short_format;
    indicator_style = none;
    all_files = false;
    really_all_files = false;
    dir_defaulted = true;
    trace_links = false;

    /* Modify switches based on user input */

    i = decode_switches(argc, argv);

    /* Initialize array of file */
    nfiles = 0x100;
    files = malloc(nfiles * sizeof(struct file));
    files_index = 0;

    clear_files();

    
    /*
        Now we have option part of argv,
        if we have more contents they mean files/directories/whatever,
        or if we don't have anything else it means to print the cwd.

        Example: ls -ml
        - Two options and done, this means to print the cwd with 'm' 'l' switches

        Example: ls -R ~/Develop
        - One option, and then a folder (or a file as we are not sure which one it is)
    */

    if (i < argc) {
        // We do have some non-option args
        dir_defaulted = false;
    }

    for (; i < argc; i++) {
        gobble_file(argv[i]);
    }

    return EXIT_SUCCESS;
}

/* long option struct for decode_switches() */
struct option long_options[] =
{
    {"all", 0, 0, 'a'},
    {"reverse", 0, 0, 'r'},
    {"almost-all", 0, 0, 'A'},
    {"file-type", 0, 0, 'F'},
    {"dereference", 0, 0, 'L'},
    {"literal", 0, 0, 'N'},
    {"quote-name", 0, 0, 'Q'},
    {"recursive", 0, 0, 'R'},
    {0, 0, 0, 0}
};

int32_t 
decode_switches(int argc, char* argv) {
    /*
        Reading each argv and modify switches
    */
    int c;
    
    while ((c = getopt_long(argc, argv, "almrtAFLSUX1", long_options, NULL)) != EOF) {
        switch (c) {
            case 'a':
                all_files = true;
                really_all_files = true;
                break;
            case 'l':
                format = long_format;
                break;
            case 'm':
                format = with_commas;
                break;
            case 'r':
                sort_reverse = true;
                break;
            case 't':
                sort_type = sort_time;
                break;
            case 'A':
                all_files = true;
                break;
            case 'F':
                indicator_style = all;
                break;
            case 'L':
                trace_links = true;
                break;
            case 'S':
                sort_type = sort_size;
                break;
            case 'U':
                sort_type = sort_none;
                break;
            case 'X':
                sort_type = sort_extension;
                break;
            case '1':
                format = one_per_line;
                break;
            default:
                // Wrong argument, print usage
                usage();
                break;
        }
    }

    /* Index in ARGV of the next element to be scanned.
    This is used for communication to and from the caller
    and for communication between successive calls to 'getopt'.

    On entry to 'getopt', zero means this is the first call; initialize.

    When 'getopt' returns -1, this is the index of the first of the
    non-option elements that the caller should itself scan.

    Otherwise, 'optind' communicates from one call to the next
    how much of ARGV has been scanned so far.  */
    return optind;
}

void 
usage() {
    printf("ls: invalid option\n");
    printf("Try 'ls --help' for more information.\n");
}

void 
clear_files() {
    for (int i = 0; i < nfiles; i++) {
        free(files[i].name);
        files[i].name = NULL;
        free(files[i].link_name);
        files[i].link_name = NULL;
    }
    files_index = 0;
}

int 
gobble_file(char* name) {
    // gobble_file runs inside a loop and each loop consumes a new filename.
    // Potentially the # of files can reach files_index,
    // and we will have to realloc the array.
    // We follow the simple strategy of doubling the space of the array
    // everytime the array reaches nfiles

    if (files_index == nfiles) {
        nfiles *= 2;
        files = realloc(files, nfiles * sizeof(struct file));
    }

    if (stat(name, &(files[files_index].stat)) == -1) {
        perror("stat failed");
        exit(EXIT_FAILURE);
    }

    /* if trace symbolic links, use stat(), else use lstat() */
    // Please note that we don't actually know whether the name is a symbolic link
    // Neither do we know whether the pointed object is valid even if it is
    // Thus we catch exception here to use lstat() if stat() fails
    // The purpose of the next block is just to load struct stat
    files[files_index].link_name = NULL;
    files[files_index].name = NULL;

    if (trace_links) {
        // We first use stat() and assume the link is valid
        if (stat(name, &(files[files_index].stat)) != 0) {
            // Then if it fails we switch to lstat()
            if (lstat(name, &(files[files_index].stat)) != 0) {
                fprintf(stderr, "lstat() failed in %s\n", __func__);
            }
        }
    }
    else {
        // Since we don't care about pointed object, just use lstat()
        if (lstat(name, &(files[files_index].stat)) != 0) {
            fprintf(stderr, "lstat() failed in %s\n", __func__);
        }
    }

    // Now struct stat has been loaded, we start to process information for symbolic links
    // S_ISLNK() returns true if file is
    #ifdef S_ISLNK
    if (S_ISLNK(files[files_index].stat.st_mode)) {
        
    }
    #endif

}

// int main(int argc, char* argv[]) {
//     /*
//         TODO: Summarize the functionality of this function
//         https://stackoverflow.com/questions/4204666/how-to-list-files-in-a-directory-in-a-c-program
//     */
    
//     // TODO: implement some flags for ls
//     DIR* d;
//     struct dirent* dir;
//     d = opendir(".");
//     int32_t fileCount = 0;
//     if (d) {
//         while ((dir = readdir(d)) != NULL) {
//             if (dir->d_type == DT_REG) {
//                 fileCount++;
//             }
//         }
//         // Now load all filenames into an array
//         // First rewind to beginning of folder
//         rewinddir(d);
//         char** filename = malloc(fileCount * sizeof(char*));
//         fileCount = 0;
//         while ((dir = readdir(d)) != NULL) {
//             if (dir->d_type == DT_REG) {
//                 int len = strlen(dir->d_name) + 1;
//                 char* n = malloc(len + 1);
//                 strncpy(n, dir->d_name, len);

//                 filename[fileCount] = n;
//                 fileCount++;
//             }
//         }
//         // Insertion sort, use strcasecmp() to compare
//         // First compare 1 with 0, obtain a stable 0~1
//         // Next compare 2 with 0~1, obtain a stable 0~2
//         // ...until compare n-1 with 0~n-2, obtain a stable 0~n-1, done
//         int32_t next = 1;
//         while (next < fileCount) {
//             int32_t walker = next;
//             while (walker > 0 && strcasecmp(filename[walker - 1], filename[walker]) > 0) {
//                 char* temp = filename[walker];
//                 filename[walker] = filename[walker - 1];
//                 filename[walker - 1] = temp;
//                 walker--;
//             }
//             next++;
//         }
//         // test print
//         for (int i = 0; i < fileCount; i++) {
//             printf("%s  ", filename[i]);
//         }
//         printf("\n");
//         // Clean up
//         for (int i = 0; i < fileCount; i++) {
//             free(filename[i]);
//         }
//         free(filename);
//         closedir(d);
//     }
//     return EXIT_SUCCESS;
// }