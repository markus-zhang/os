#ifndef _MASH_H_
#define _MASH_H_

/**
 * @file        mash.h
 * @author      Markus Zhang
 * @brief       Header file for the shell
 * @version     0.01
 * @date        2023-04-23
 * 
 * @copyright   CopyLeft baby
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define __STDC_WANT_LIB_EXT1__ 1
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "mashtool.h"

#define DEBUG                   true
#define RUNNING                 true
// Allow 64 strings in path initially, add 16 each time if needed
#define PATH_INITIAL_SIZE       64
#define PATH_INCREMENT_SIZE     16
#define CWD_LEN                 256
#define ARGS_INITIAL_SIZE       64
#define ARGS_INCREMENT_SIZE     16

/**
 * @brief functionalities:
 *  - pipe: 0 indicates no pipe, 1 indicates sender, and 2 indicates both
 *      So for cat ./file.txt | grep master | wc    
 *      "cat" would have pipe = 1, "grep" has pipe = 2, and "wc" has pipe = 0
 *  - redir: NULL indicates no redirection, otherwise it's the path of the file
 *      So for echo "dsdsdsd sd" "sdsdsds" > ~/develop/temp/ourput.txt,
 *      command_args will have 3 elements, num_args = 3,
 *      redir points to the string "~/develop/temp/output.txt"
 */
struct command {
    char** command_args;
    int num_args;
    char* redir;
    int pipe;
};

/**
 * @brief The enum is for mash_split_command to pass info to make_command()
 * It indicates whether the current command _TO BE EXTRACTED_ is followed by >, & or |
 * Or NONE if followed by nothing
 * 
 */
enum cli_special {
    NONE = 0,
    REDIR,
    PARALLEL,
    PIPE
};

/*!
    Global variables
    - cwd: current working directory
        - type: C string
    - path: collection of all directories in $PATH
        - type: array of C strings
    - pathCount: count of paths in $PATH
        - type: int32_t
        - comment: changes for each remove/add operation
*/
char*           cwd;
char**          path;
int32_t         pathCount;
char**          args;
int32_t         argsindex;
int32_t         status_split_command;
int32_t         status_run;
char*           line;
// eventually all commands get parsed into this array
struct command* all_commands[64] = {NULL};
int32_t         command_index;
// for &
bool            switch_parallel = false;
// for |
bool            switch_pipe = false;


int32_t init_path();
int32_t inspect_path();
int32_t add_path(const char* newPath);
int32_t remove_path(const char* oldPath);
int32_t process_command(char* command);
int32_t _internal_ls();
void print_args();
int builtin(char* program);
void print_cd_usage();

void mash_split_line(char* line);
int mash_split_command();
void make_command(int command_start, int command_end, int* command_index, int lookback, enum cli_special special);
int mash_execute(struct command*[]);


#endif