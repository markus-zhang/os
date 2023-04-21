#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int pipefd[2];
    int pipefd_next[2];
    pid_t pid_A, pid_B, pid_C;

    // Create a pipe
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }
    
    if (pipe(pipefd_next) == -1) {
        perror("second pipe");
        exit(EXIT_FAILURE);
    }

    // Fork child process A
    pid_A = fork();
    if (pid_A == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid_A == 0) { // Child process A
        // Redirect stdout to the pipe's write end
        dup2(pipefd[1], STDOUT_FILENO);

        // Close the read end, as we don't need it in this process
        close(pipefd[0]);

        // Execute "cat ./file.txt"
        execlp("echo", "echo", "asdsasdsadasdsadsad cat", (char *)NULL);
        perror("execlp");
        exit(EXIT_FAILURE);
    }

    // Fork child process B
    pid_B = fork();
    if (pid_B == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid_B == 0) { // Child process B
        // Redirect stdin to the pipe's read end
        dup2(pipefd[0], STDIN_FILENO);

        // Close the write end, as we don't need it in this process
        close(pipefd[1]);
        
        dup2(pipefd_next[1], STDOUT_FILENO);
        close(pipefd_next[0]);

        // Execute "grep password"
        execlp("grep", "grep", "cat", (char *)NULL);
        perror("execlp");
        exit(EXIT_FAILURE);
    }
    
    // Fork child process C
    pid_C = fork();
    if (pid_C == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid_C == 0) { // Child process B
        // Redirect stdin to the pipe's read end
        dup2(pipefd_next[0], STDIN_FILENO);

        // Close the write end, as we don't need it in this process
        close(pipefd_next[1]);

        // Execute "grep password"
        execlp("wc", "wc", (char *)NULL);
        perror("wc");
        exit(EXIT_FAILURE);
    }

    // Close both ends of the pipe in the parent process
    close(pipefd[0]);
    close(pipefd[1]);
    close(pipefd_next[0]);
    close(pipefd_next[1]);

    // Wait for both child processes to complete
    waitpid(pid_A, NULL, 0);
    waitpid(pid_B, NULL, 0);
    waitpid(pid_C, NULL, 0);

    return 0;
}
