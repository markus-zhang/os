/*
    Explore the global variable environ
    7.5 APUE, page 238 of 1034

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char* argv[]) {
    for (int i = 0; __environ[i] != NULL; i++) {
        printf("%s\n", __environ[i]);
    }
    exit(EXIT_SUCCESS);
}