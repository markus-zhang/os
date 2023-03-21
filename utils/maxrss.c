/*
    A simple util that gets maximum rss spent of a child process
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
    int status = 0;
    int pid = fork();

    if (pid == 0) {
        // Child process
        char* buffer = malloc(1024*1024*100);   // Allocate 100MB of memory
        for (int i = 0; i < 1024*1024*100; i++) {
            buffer[i] = 'a';
        }
        printf("\n");
        free(buffer);
        exit(EXIT_SUCCESS);
    }
    else if (pid > 0) {
        // Parent process
        // https://stackoverflow.com/questions/19461744/how-to-make-parent-wait-for-all-child-processes-to-finish
        // If we don't wait, because the program runs concurrently, this block always print 0KB
        while ((wait(&status)) > 0);
        struct rusage usage;
        getrusage(RUSAGE_CHILDREN, &usage);
        printf("Maximum memory usage of child process: %ld KB\n", usage.ru_maxrss);
    }
    else {
        // Error
        fprintf(stderr, "fork() error");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}