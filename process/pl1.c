#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main(int argc, char* argv[]) {
    printf("hello world (pid:%d)\n", (int)getpid());
    int rc = fork();
    if (rc < 0) {
        // fork error
        fprintf(stderr, "fork() failed\n");
        exit(EXIT_FAILURE);
    }
    else if (rc == 0) {
        // child process
        printf("hello, I am child (pid:%d)\n", (int)getpid());
        char* myargs[3];
        // The array of pointers must be terminated by a NULL pointer
        // https://linux.die.net/man/3/execvp
        myargs[0] = strdup("wc");
        myargs[1] = strdup("pl1.c");
        myargs[2] = NULL;
        if (execvp(myargs[0], myargs) == -1) {
            printf("this shouldn't print out as control returns to parent process after execvp\n");
        }
    }
    else {
        // parent process
        int rc_wait = wait(NULL);
        printf("hello, I am parent of %d (rc_wait:%d) (pid:%d)\n", rc, rc_wait, (int)getpid());
    }
    exit(EXIT_SUCCESS);
}