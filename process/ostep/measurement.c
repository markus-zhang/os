#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>

#define nloop 1000

void timediff(struct timeval ty, struct timeval ty_2, int loop);

int main(int argc, char** argv) {
    struct timeval ty;
    struct timeval ty_2;

    if (gettimeofday(&ty, NULL) < 0) {
        perror("gettimeofday()");
        exit(1);
    }

    printf("Miliseconds since Epoch: %ld\n", ty.tv_usec);
    printf("Seconds since Epoch: %ld\n", ty.tv_sec);

    // run a 0-byte read() system call
    int fd = 0;
    if ((fd = open("./measurement.c", O_RDONLY)) < 0) {
        perror("open system call error");
    }

    char buf[1];

    gettimeofday(&ty, NULL);
    for (int i = 0; i < nloop; i++) {
        read(fd, buf, 0);
    }
    gettimeofday(&ty_2, NULL);

    timediff(ty, ty_2, nloop);

    return 0;
}

void timediff(struct timeval ty, struct timeval ty_2, int loop) {
    int milisecond = ty_2.tv_usec - ty.tv_usec;
    long int sec = ty_2.tv_sec - ty.tv_sec;
    if (milisecond < 0) {
        milisecond = 1000000 - milisecond;
        sec -= 1;
    }
    printf("Time difference for %d loops: %ld second(s), %d milisecond(s)\n", nloop, sec, milisecond);
}