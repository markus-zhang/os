#include "stdio.h"
#include "stdlib.h"
#include <unistd.h>
#include <sys/stat.h>

#define buffer_size 1024

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: ./wcat [filename]\n");
        exit(1);
    }
    char* filename = argv[1];
    // Test stat() to grab file information
    struct stat sb;

    if (stat(argv[1], &sb) == -1) {
        perror("stat");
        exit(EXIT_FAILURE);
    }

    printf("File size: %ld\n", sb.st_size);

    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("wcat: cannot open file %s\n", filename);
        exit(2);
    }
    else {
        char* buffer = malloc(buffer_size * sizeof(char));
        fread(buffer, sb.st_size, 1, fp);
        printf("%s\n", buffer);
        fclose(fp);
        free(buffer);
    }
    exit(0);
}