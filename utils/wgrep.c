/*
    Usage: ./wgrep [string] [filename]

    Algorithm:
    Program reads filename by line using getline();
    If [string] has a match in the line, print the line;
    Otherwise skip to next line
*/

#include "stdio.h"
#include "stdlib.h"
#include "stdbool.h"
#include "string.h"

bool hasString(char* hostString, char* subString);

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: ./wgrep [string] [filename]\n");
        exit(1);
    }
    char* string = argv[1];
    char* filename = argv[2];
    FILE* fp = fopen(filename, "r");
    if (fp == NULL) {
        printf("Cannot open file %s\n", filename);
        exit(2);
    }
    else {
        char* buffer = NULL;
        size_t lineLength = 80;
        while (getline(&buffer, &lineLength, fp) != -1) {
            if (hasString(buffer, string)) {
                printf("%s", buffer);
            }
        }
        free(buffer);
        fclose(fp);
    }
    exit(EXIT_SUCCESS);
}

bool hasString(char* hostString, char* subString) {
    if (strlen(subString) == 0) {
        return true;
    }
    return (strstr(hostString, subString) != NULL);
}