/*
    A minimum shell implementation for study of processes under Linux
    gcc -std=gnu99 -Wall -Werror -pedantic mash.c -o mash
*/

#include "mash.h"

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
        // exit(0);
        status_split_command = mash_split_command();
        // exit(0);
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
    enum parsestat {
        ready = 0,
        token_begin,
        in_string
    };

    argsindex = -1;
    int len = strlen(line);
    enum parsestat stat = ready;
    int token_start = 0;
    int token_end = 0;

    // loop stops at [len-2] to get rid of linebreak
    for (int i = 0; i <= len - 1; i++) {
        char current_char = line[i];
        char next_char = line[i + 1];

        // Starting a new token
        if (current_char != ' ' && current_char != '"' && current_char != '\0' && current_char != '\n') {
            if (stat == ready) {
                stat = token_begin;
                token_start = i;
            }
        }
        // Ending a token NOT encapsulated by quotation marks
        else if (current_char == ' ' || current_char == '\0' || current_char == '\n') {
            // If ready, do nothing; If already parsing, set tokenend (if in string then stat is in_string)
            // If in string, do nothing; if is_escaped, do nothing
            if (stat == token_begin) {
                token_end = i - 1;
                int token_len = token_end - token_start + 1;
                char* token = malloc(token_len + 1);
                if (token == NULL) {
                    perror("malloc");
                    return;
                }
                strncpy(token, line + token_start, token_len);
                token[token_len] = '\0';

                argsindex++;
                args[argsindex] = token;
                stat = ready;
            }
        }
        // Handling double quotes
        else if (current_char == '"') {
            if (stat == ready) {
                if (next_char != '\0') {
                    token_start = i + 1;
                    stat = in_string;
                }
                else {
                    fprintf(stderr, "Wrong parsing status at index %d: %c\n", i, line[i]);
                    return;
                }
            }
            else if (stat == in_string) {
                // OK we are out
                token_end = i - 1;
                int token_len = token_end - token_start + 1;
                char* token = malloc(token_len + 1);
                if (token == NULL) {
                    perror("malloc");
                    return;
                }
                strncpy(token, line + token_start, token_len);
                token[token_len] = '\0';
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
    /**
     * @brief We need a new algorithm to allow commands such like:
     * cmdA1 | cmdA2 ... | cmdAn & cmdB1 | ... | cmdBm
     * 
     */
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
                make_command(command_start, command_end, &command_index, 0, REDIR);
                
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
                switch_pipe = false;
                make_command(command_start, command_end, &command_index, 1, PARALLEL);

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
        else if (strlen(args[command_end]) == 1 && strncmp(args[command_end], "|", 1) == 0) {
            if (command_end == 0) {
                // | cannot be the first arg
                fprintf(stderr, "| cannot be the first command\n");
                exit(EXIT_FAILURE);
            }
            if (stat == READY) {
                // READY example: echo "sdsd" > blah.txt | sort
                //      - stat is READY after parsing '|'
                //      - this is wrong as > cannot be followed by a |
                fprintf(stderr, "| cannot follow >\n");
                exit(EXIT_FAILURE);
            }
            else if (stat == IN_COMMAND) {
                // IN_COMMAND example: cat blah.txt | sort
                //      - stat is IN_COMMAND when hits '|'
                //      - this is legal
                //      We should just use one pipe flag and let execute() to
                //      figure out the direction of pipes
                
                switch_pipe = true;
                make_command(command_start, command_end, &command_index, 1, PIPE);

                if (command_index >= 64) {
                    // Only allocated 64 struct command*
                    break;
                }
                command_end++;
                command_start = command_end;
                stat = READY;
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
            make_command(command_start, command_end, &command_index, 1, NONE);
            break;
        }
    }
    printf("Parsing ended\n");
    // Debug: print all args
    
    for (int i = 0; i < command_index; i++) {
        printf("%s: ", all_commands[i]->command_args[0]);
        char** a = all_commands[i]->command_args;
        for (int j = 0; j < all_commands[i]->num_args; j++) {
            printf("%s ", a[j]);
        }
        if (all_commands[i]->redir != NULL) {
            printf("\n redir target: %s", all_commands[i]->redir);
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

    if (switch_parallel) {
        // TODO: Run all commands parallelly
        printf("Parallel execution\n");
        pid_t pid;
        int status;
        for (int i = 0; i < command_index; i++) {
            pid = fork();
            if (pid == 0) {
                // Child process
                // Deal with redirection
                struct command* current_command = all_commands[i];
                char* redir = current_command->redir;
                if (redir != NULL) {
                    // need to redirect output to file
                    printf("Ready to redir to: %s\n", redir);
                    int fd1 = creat(redir, 0644);
                    if (fd1 < 0) {
                        fprintf(stderr, "creat() error: at line %d in %s\n", __LINE__, __func__);
                        exit(EXIT_FAILURE);
                    }
                    dup2(fd1, STDOUT_FILENO);
                    close(fd1);
                }
                if (execvp(current_command->command_args[0], current_command->command_args) == -1) {
                    fprintf(stderr, "execvp() failed: at line %d in %s\n", __LINE__, __func__);
                    exit(EXIT_FAILURE);
                }
                exit(EXIT_FAILURE);
            }
            else if (pid < 0) {
                // Error forking
                fprintf(stderr, "fork() failed: at line %d in %s\n", __LINE__, __func__);       
            }
            else {
                // Parent process, do nothing for parallel runs
            }
        }
        // TODO: need to wait each pid here
        waitpid(pid, &status, WUNTRACED);
        return 0;
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
                // Deal with redirection
                struct command* current_command = all_commands[i];
                char* redir = current_command->redir;
                if (redir != NULL) {
                    // need to redirect output to file
                    printf("Ready to redir to: %s\n", redir);
                    int fd1 = creat(redir, 0644);
                    if (fd1 < 0) {
                        fprintf(stderr, "creat() error: at line %d in %s\n", __LINE__, __func__);
                        exit(EXIT_FAILURE);
                    }
                    dup2(fd1, STDOUT_FILENO);
                    close(fd1);
                }
                if (execvp(current_command->command_args[0], current_command->command_args) == -1) {
                    fprintf(stderr, "execvp() failed: at line %d in %s\n", __LINE__, __func__);
                    exit(EXIT_FAILURE);
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
    }

    // check if program is a built-in command
    // we have three of them: exit, cd and path

    
    // char* program = args[0];
    return 0;
}

/**
 * @brief   Seperate the logic of making a command into a function
 * 
 * @param   command_index: the index of the command in all_commands[]
 * @param   command_start: starting index of a command, which is the execution program, arg[0] 
 * @param   command_end: ending index of a command, which is the last arg "token"
 * @param   lookback: a special switch, 1 for grabbing the last command if it is an "ordinary"
 *                  command (without > or not prefixed by &), 0 for other cases
 *                  See make_command() calls for more information
 *          struct command {
 *              char** command_args;
 *              int num_args;
 *              char* redir;
 *              int pipe;
 *          };
 */
void
make_command(int command_start, int command_end, int* command_index, int lookback, enum cli_special special) {
    // The +2 at the end is to 
    // 1) contain a NULL char& for execvp()
    // 2) contain the executable file name, again for execvp()
    // See "man execvp" for more information
    char* redir = NULL;
    int pipe = 0;
    if (switch_pipe) {
        pipe = 1;
    }
    int redir_lookback = 2; // args won't take ">" and "file"
    if (special != REDIR) {
        redir_lookback = 0;
    }
    int command_args_size = command_end - command_start - lookback - redir_lookback + 2;
    char** command_args = malloc(sizeof(char*) * command_args_size);

    // loop will fill command_args[0] ~ command_args[the second last]    
    for (int i = command_start; i <= command_start + command_args_size - 2; i++) {
        char* arg = malloc(strlen(args[i]) + 1);
        strncpy(arg, args[i], strlen(args[i]));
        arg[-1] = '\0';
        command_args[i-command_start] = arg; 
    }
    // Insert a NULL into command_args at the end
    command_args[command_args_size - 1] = NULL;
    if (redir_lookback > 0) {
        redir = malloc(strlen(args[command_end]) + 1);
        strncpy(redir, args[command_end], strlen(args[command_end]));
        redir[-1] = '\0';
    }

    struct command* command_next = malloc(sizeof(struct command));
    command_next->command_args = command_args;
    command_next->num_args = command_args_size - 1;
    command_next->redir = redir;
    command_next->pipe = pipe;
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