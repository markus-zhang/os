/*
    Usage: ./reverse [input] [output]

    Algorithm:
    Builds a linked list of strings, each for a line in [input],
    and then walk the nodes backward to build the [output]
*/

#include "stdio.h"
#include "stdlib.h"
#include <stdint.h>
#include <stdbool.h>

typedef struct lineNode {
    char* line;
    lineNode* next;
    lineNode* prev;
} lineNode;

typedef struct lineList {
    lineNode* head;
    lineNode* tail;
} lineList;

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: ./wunzip [filename]\n");
        exit(EXIT_FAILURE);
    }
    char* filename = argv[1];
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Cannot open file %s\n", filename);
        exit(EXIT_FAILURE);
    }

    // fseek() to the end of file and move back to find EOL
    if (fseek(fp, -1, SEEK_END) != 0) {
        fprintf(stderr, "fseek() error\n");
    }
    

    exit(EXIT_SUCCESS)
}