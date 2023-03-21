#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <time.h>

int main(int argc, char* argv[]) {
    printf("hello world (pid:%d)\n", (int)getpid());
    int rc = fork();
    close(STDOUT_FILENO);
    open("./p4.output", O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);

    if (rc < 0) {
        // fork error
        fprintf(stderr, "fork() failed\n");
        exit(EXIT_FAILURE);
    }
    else if (rc == 0) {
        // sleep a bit until parent changes output target
        usleep(1000000);
        // child process
        printf("hello, I am child (pid:%d)\n", (int)getpid());

        // we want to immitate redirection >
        // we should close STDOUT and open a file
        // Linux looks for free file descriptors starting from 0, which is closed, 
        // the next availale one is STDOUT_FILENO, thus gets assigned
        
        // close(STDOUT_FILENO);
        // open("./p4.output", O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);

        // The array of pointers must be terminated by a NULL pointer
        // https://linux.die.net/man/3/execvp
        char* myargs[3];
        myargs[0] = strdup("wc");
        myargs[1] = strdup("pl1.c");
        myargs[2] = NULL;
        printf("This is a test.       Does it show up in file or stdout?\n");
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