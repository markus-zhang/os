#include "stdio.h"
#include "stdlib.h"

#define buffer_size 1024

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: ./wcat [filename]\n");
        exit(1);
    }
    char* filename = argv[1];
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Cannot open file %s\n", filename);
        exit(2);
    }
    else {
        char* buffer = malloc(buffer_size * sizeof(char));
        while (fgets(buffer, buffer_size, fp) != NULL) {
            printf("%s\n", buffer);
        }
        fclose(fp);
    }
    exit(0);
}