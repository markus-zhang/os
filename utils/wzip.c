/*
    Usage: ./wzip [input] > [output]

    Algorithm:
    Let's say we have aaaaabbbc
    This will be encoded as 5a3b1c:
    - each integer is strictly 4-byte
    - each character is 1-byte
    So in binary format this becomes 
*/

#include "stdio.h"
#include "stdlib.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct run {
    int32_t rep;
    char ch;
} run;

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
        char ch = (char)fgetc(fp);
        int32_t rep = 1;
        while (true) {
            char chWalker = (char)fgetc(fp);
            if (chWalker == ch) {
                rep++;
            }
            else {
                // char changes, print information
                // printf("%c, repeated for %d times\n", ch, rep);
                run r = {.ch = ch, .rep = rep};
                fwrite(&r, 5, 1, stdout);
                ch = chWalker;
                rep = 1;
            }
            if (chWalker == EOF) {
                break;
            }
        }
        fclose(fp);
    }
    exit(0);
}