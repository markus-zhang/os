#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: mwrite ttyname\n");
        exit(1);
    }

    int fd;
    char buffer[BUFSIZ];

    // open devices
    if ((fd = open(argv[1], O_WRONLY)) == -1) {
        perror(argv[1]);
        exit(1);
    }

    // write to device
    while (fgets(buffer, BUFSIZ, stdin) != NULL) {
        if (write(fd, buffer, strlen(buffer)) == -1) {
            break;
        }
    }

    close(fd);

    return 0;
}