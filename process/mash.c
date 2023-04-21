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
#include <sys/types.h>
#include <sys/wait.h>
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
#define ARGS_INITIAL_SIZE       64
#define ARGS_INCREMENT_SIZE     16

struct command {
    char* command_execution;
    char** command_args;
    int num_args;
};

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
void make_command(int command_start, int command_end, int* command_index, int lookback);
int mash_execute(struct command*[]);

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
    line = NULL;
    args = malloc(ARGS_INITIAL_SIZE * sizeof(char*));
    status_run = 1;
    status_split_command = 0;

    while(status_run) {
        fprintf(stdout, ">");
        size_t size;
        if (getline(&line, &size, stdin) == -1) {
            if (feof(stdin)) {
                // Ctrl-D (EOF)
                printf("Terminated by user, bye!\n");
                exit(EXIT_SUCCESS);
            }
            fprintf(stderr, "getline() error in %s\n", __func__);
            exit(EXIT_FAILURE);
        }
        /**
         * @brief Process user input
         *  - built-int command:
         *      - exit: call `exit` system call with 0 as a parameter
         *      - cd: always take one arg. Use `chdir()` system call
         *      - path: takes >= 0 args, each separated by space, added to search path of the shell
         * 
         */

        mash_split_line(line);
        // print_args();
        status_split_command = mash_split_command();
        status_run = mash_execute(all_commands);
        status_run = 0;
    }

    // shut down
    for (int i = 0; i < pathCount; i++) {
        free(path[i]);
        path[i] = NULL;
    }

    free(line);
    for (int i = 0; i <= argsindex; i++) {
        free(args[i]);
    }

    for (int i = 0; i <= command_index - 1; i++) {
        // free(all_commands[i]->command_execution);
        for (int j = 0; j < all_commands[i]->num_args; j++) {
            free(all_commands[i]->command_args[j]);
        }
        free(all_commands[i]->command_args);
        free(all_commands[i]);
    }

    free(cwd);
    free(path);
    free(args);
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
    // inspect_path();
    return EXIT_SUCCESS;
}

int32_t 
inspect_path() {
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

    if ((strncmp(command, "cd", 2) == 0) && (strlen(command) == 3)) {
        return _internal_ls();
    }
    
    return EXIT_SUCCESS;
}

/**
 * @brief split line into tokens: 
 *      1) quote pairs define one token
 * 
 * @param line 
 * @return char** 
 */
void
mash_split_line(char* line) {
    argsindex = -1;
    // char** argswalker = args;
    int len = strlen(line);
    enum parsestat {
        ready = 0,
        token_begin,
        in_string
    };

    enum parsestat stat = ready;
    int tokenstart = 0;
    int tokenend = 0;

    // loop stops at [len-2] to get rid of linebreak
    for (int i = 0; i <= len - 1; i++) {
        if (line[i] != ' ' && line[i] != '"' && i < len - 1) {
            if (stat == ready) {
                stat = token_begin;
                tokenstart = i;
            }
        }
        else if (line[i] == ' ' || i == len - 1) {
            // If ready, do nothing; If already parsing, set tokenend (if in string then stat is in_string)
            // If in string, do nothing; if is_escaped, do nothing
            if (stat == token_begin) {
                tokenend = i - 1;
                char* token = malloc(tokenend - tokenstart + 2);
                for (int j = tokenstart; j <= tokenend; j++) {
                    token[j - tokenstart] = line[j];
                }
                token[tokenend - tokenstart + 1] = '\0';
                argsindex++;
                args[argsindex] = token;
                stat = ready;
            }
        }
        else if (line[i] == '"') {
            if (stat == ready) {
                if (line[i+1] != '\0') {
                    tokenstart = i + 1;
                    stat = in_string;
                }
                else {
                    fprintf(stderr, "Wrong parsing status at index %d: %c\n", i, line[i]);
                    exit(EXIT_FAILURE);
                }
            }
            else if (stat == in_string) {
                // OK we are out
                tokenend = i - 1;
                char* token = malloc(tokenend - tokenstart + 2);
                for (int j = tokenstart; j <= tokenend; j++) {
                    token[j - tokenstart] = line[j];
                }
                token[tokenend - tokenstart + 1] = '\0';
                argsindex++;
                args[argsindex] = token;
                stat = ready;
            }
        }
    }
    printf("argsindex: %d\n", argsindex);
}

int
mash_split_command() {
    enum command_stat {
        READY = 0,
        IN_COMMAND
    };
    enum command_stat stat = READY;
    int command_start = 0;
    int command_end = 0;
    command_index = 0;
    switch_parallel = false;

    while (command_end <= argsindex) {
        if (strlen(args[command_end]) == 1 && strncmp(args[command_end], ">", 1) == 0) {
            if (stat != IN_COMMAND) {
                fprintf(stderr, "Parsing args error: hits > but not parsing commands\n");
                return EXIT_FAILURE;
            }
            else {
                command_end ++;
                if (command_end > argsindex) {
                    fprintf(stderr, "Parsing args error: no arg after >\n");
                    return EXIT_FAILURE;
                }
                make_command(command_start, command_end, &command_index, 0);
                
                if (command_index >= 64) {
                    // Only allocated 64 struct command*
                    break;
                }
                command_end++;
                command_start = command_end;
                stat = READY;
            }
        }
        else if (strlen(args[command_end]) == 1 && strncmp(args[command_end], "&", 1) == 0) {
            if (command_end == 0) {
                // & cannot be the first arg
                exit(EXIT_FAILURE);
            }            
            if (stat == READY) {
                // READY example: echo "blah" > blah.txt & ls /dev
                //      - stat is READY after parsing '>'
                switch_parallel = true;
                command_end++;
                command_start = command_end;
            }
            else if (stat == IN_COMMAND) {
                // IN_COMMAND example: cat blah.txt & ls /dev
                //      - stat is IN_COMMAND when hits '&'
                switch_parallel = true;
                make_command(command_start, command_end, &command_index, 1);

                if (command_index >= 64) {
                    // Only allocated 64 struct command*
                    break;
                }
                command_end++;
                command_start = command_end;
                stat = READY;
            }
            else {
                fprintf(stderr, "& must be in READY or IN_COMMAND stat\n");
                return EXIT_FAILURE;
            }
        }
        else {
            // an ordinary string
            if (stat == READY) {
                stat = IN_COMMAND;
            }
            command_end++;
        }
        /**
         * @brief IMPORTANT!
         * If stat == IN_COMMAND but command_end > argsindex
         * this means we are reaching end of args array
         * but the last one is not parsed yet so we need to store it again
         * TODO: It probably make s alot of sense to separate the storage part
         * into a function
         */
        if (stat == IN_COMMAND && command_end > argsindex) {
            make_command(command_start, command_end, &command_index, 1);
            break;
        }
    }
    printf("Parsing ended\n");
    // Debug: print all args
    
    for (int i = 0; i < command_index; i++) {
        printf("%s: ", all_commands[i]->command_execution);
        char** a = all_commands[i]->command_args;
        for (int j = 0; j < all_commands[i]->num_args; j++) {
            printf("%s ", a[j]);
        }
        printf("\n-----------------------------\n");
    }
    
    // Force quit
    // exit(EXIT_SUCCESS);
    return EXIT_SUCCESS;
}

void
print_args() {
    for (int i = 0; i <= argsindex; i++) {
        printf("%s\n", args[i]);
    }
}

int
mash_execute(struct command* all_commands[]) {
    // command_index should be at least 1 (only 1 command)
    if (command_index <= 0) {
        return 0;
    }

    if (!switch_parallel) {
        // TODO: Run all commands parallelly
    }
    else {
        // Then we should only have 1 command
        // But run them sequentially anyway in case we add that functionality
        // e.g. maybe dans futur we will add ";", or "&&"
        // Test with just one command
        pid_t pid;
        int status;
        for (int i = 0; i < command_index; i++) {
            pid = fork();
            if (pid == 0) {
                // Child process
                if (execvp(all_commands[i]->command_execution, all_commands[i]->command_args) == -1) {
                    perror("mash");
                }
                exit(EXIT_FAILURE);
            }
            else if (pid < 0) {
                // Error forking
                fprintf(stderr, "fork() failed: at line %d in %s\n", __LINE__, __func__);       
            }
            else {
                // Parent process
                do {
                    waitpid(pid, &status, WUNTRACED);
                } while (!WIFEXITED(status) && !WIFSIGNALED(status));
            }
        }
        return 1;
        for (int i = 0; i < command_index; i++) {
            // TODO: Implement this part after the test is done above
        }
    }

    // check if program is a built-in command
    // we have three of them: exit, cd and path

    
    // char* program = args[0];
    
    /**
     * @brief TODO: scan for redirection and parallel commands
     * The logic should look like this:
     *
     *  struct command {
     *      char* command_execution;
     *      char** command_args; 
     *  };
     *  set stat = ready;
     *  set command_start = 0;
     *  set command_end = 0;
     *  set command_index = 0;
     *  struct command all_commands[64];
     *  while (command_start <= argsindex):
     *      set command_next = malloc(sizeof(struct command));
     *      set switch_redir = False;
     *      check args[command_start];
     *      if (a '>' has been hit):
     *          if (stat != IN_COMMAND):
     *              exit ERROR;
     *          else:
     *              set command_end = command_end + 1;  // should have another argument after redirection
     *              if (command_end > argsindex):
     *                  exit ERROR; // missing argument after redirection
     *              set char** command_args = malloc(sizeof(char*) * (command_end - command_start));
     *              for (each char* in args from command_start + 1 to command_end):
     *                  copy into command_args;
     *              set char* command_executable = malloc(strlen(args[command_start]) + 1);
     *              copy args[command_start] which is the name of execution into command_executable;
     *              
     *              // Fill the next command and put it into the command array
     *              set command_next.command_execution = command_execution;
     *              set command_next.command_args = command_args;
     *              set all_commands[command_count++] = command_next;
     *              if (command_index >= 64):
     *                  return;     // Ignore the rest
     *
     *              // Reset command_start and command_end
     *              command_end++;
     *              set command_start = command_end;
     *              set stat = READY;
     *      else if (a '&' has been hit):
     *          if (command_end == 0):
     *              exit ERROR;     // First args cannot be &
     *          if (stat != READY || stat != IN_COMMAND):
     *              // READY example: echo "blah" > blah.txt & ls /dev
     *              //      - stat is READY after parsing '>'
     *              // IN_COMMAND example: cat blah.txt & ls /dev
     *              //      - stat is IN_COMMAND when hits '&'
     *              exit ERROR;     // Is this necessary?
     *          if (stat == READY):
     *              set switch_parallel = True;
     *              increment command_start and command_end to next token;
     *              set stat = READY;
     *              otherwise do nothing as the previous command was already inserted
     *          else if (stat == IN_COMMAND):
     *              grab the previous command similar to we did for '>'
     *              set switch_parallel = True;
     *              increment command_start and command_end to next token;
     *              set stat = READY;
     *      else if (an ordinary string has been hit):
     *          if (stat == READY):
     *              set stat = IN_COMMAND;
     *          else if (stat == IN_COMMAND):
     *              do nothing
     */

    /*
    if (builtin(program) == 2) {
        // Not a builtin
        printf("Not a builtin.\n");
        return 1;
    }
    */

    return 0;
}

/**
 * @brief Seperate the logic of making a command into a function
 * 
 * @param command_start: starting index of a command, which is the execution program, arg[0] 
 * @param command_end: ending index of a command, which is the last arg "token"
 * @param command_index: the index of the command in all_commands[]
 * @param lookback: a special switch, 1 for grabbing the last command if it is an "ordinary"
 *                  command (without > or not prefixed by &), 0 for other cases
 *                  See make_command() calls for more information
 */
void
make_command(int command_start, int command_end, int* command_index, int lookback) {
    // The +2 at the end is to 
    // 1) contain a NULL char& for execvp()
    // 2) contain the executable file name, again for execvp()
    // See "man execvp" for more information
    char** command_args = malloc(sizeof(char*) * (command_end - command_start - lookback + 2));

    // loop will fill command_args[0] ~ command_args[the second last]
    for (int i = command_start; i <= command_end - lookback; i++) {
        char* arg = malloc(strlen(args[i]) + 1);
        strncpy(arg, args[i], strlen(args[i]));
        arg[-1] = '\0';
        // Starting from index 1 and index 0 is left for execution file
        command_args[i-command_start] = arg; 
    }
    // Insert a NULL into command_args
    command_args[command_end - command_start - lookback + 1] = NULL;
    char* command_executable = malloc(strlen(args[command_start]) + 1);
    if (strncpy(command_executable, args[command_start], strlen(args[command_start])) == NULL) {
        fprintf(stderr, "strncpy() error in %s\n", __func__);
        return;
    }
    command_executable[-1] = '\0';
    // argv[] must have execution file as first element
    // command_args[0] = command_executable;

    struct command* command_next = malloc(sizeof(struct command));
    command_next->command_execution = command_executable;
    command_next->command_args = command_args;
    command_next->num_args = command_end - command_start - lookback + 1;
    all_commands[*command_index] = command_next;
    (*command_index)++;    // keep consistency to use i < command_index
}

int 
builtin(char* program) {
    // Control characters
    if (strlen(program) == 4 && strncmp(program, "exit", 4) == 0) {
        // just exit
        printf("Built-in command: exit\n");
        exit(EXIT_SUCCESS);
    }
    else if (strlen(program) == 2 && strncmp(program, "cd", 2) == 0) {
        if (argsindex == 0) {
            return EXIT_SUCCESS;
        }
        else if (argsindex >= 1 && strlen(args[1]) > 0) {
            int n = chdir(args[1]);
            if (n == 0) {
                char* buf = malloc(256);
                if (getcwd(buf, 256) == NULL) {
                    fprintf(stderr, "getscwd() error in %s.\n", __func__);
                    exit(EXIT_FAILURE);
                }
                else {
                    printf("cwd: %s\n", buf);
                    free(buf);
                }
            }
            else {
                fprintf(stderr, "chdir() error in %s.\n", __func__);
            }
            return n;
        }
        else {
            print_cd_usage();
            return EXIT_FAILURE;
        }
    }
    else if (strlen(program) == 4 && strncmp(program, "path", 4) == 0) {
        /**
         * @brief if no extra arg then display all paths
         * otherwise add into path
         * 
         */
        int32_t status = 0;
        if (argsindex == 0) {
            return inspect_path();
        }
        else {
            for (int i = 1; i <= argsindex; i++) {
                status = add_path(args[argsindex]);
                if (status != 0) {
                    fprintf(stderr, "add_path() failed\n");
                }
            }
            inspect_path();
        }
        return status;
    }
    else if (strlen(program) == 4 && strncmp(program, "mcat", 4) == 0) {
        // Added mcat for testing redirection

    }
    return 2;   // Not a builtin
}

void
print_cd_usage() {
    printf("Usage: cd [arg]\n");
}

void
print_path_usage() {
    printf("Usage: path [arg1] [arg2] ...\n");
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