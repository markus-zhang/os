/*
    Usage: ./wunzip [input] > [output]
    The program will generate a text file in the same folder

    Algorithm:
    We read every 5 bytes and extract the first 4 bytes as an integer "num",
    the highest 1 byte as a char "char", and print "char" "num" times to stdout.
    The redirection > writes content into [output]
*/

#include "stdio.h"
#include "stdlib.h"
#include <stdint.h>
#include <stdbool.h>

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

typedef struct run {
    int32_t rep;
    char ch;
} run;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: ./wunzip [filename]\n");
        exit(1);
    }
    char* filename = argv[1];
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Cannot open file %s\n", filename);
        exit(2);
    }
    else {
        unsigned char bufferNum[5];
        unsigned char bufferChar = 0;
        size_t ret = 0;
        while ((ret = fread(bufferNum, sizeof(*bufferNum), ARRAY_SIZE(bufferNum), fp)) == 5) {
            // Cast to int* to cut off the high byte
            int32_t num = *(int*)bufferNum;
            bufferChar = bufferNum[4];
            for (int i = 0; i < num; i++) {
                // We rely on redirection > to write to file
                printf("%c", bufferChar);
            }
        }

        fclose(fp);
    }
    exit(0);
}