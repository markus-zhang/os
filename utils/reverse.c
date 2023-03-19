/*
    Usage: ./reverse [input] [output]

    Algorithm:
    https://stackoverflow.com/questions/6922829/read-file-backwards-last-line-first

    Notes:
    - Make sure to print first line
    - Take care to not exclude edge cases (line only contains '\n' for example)
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <errno.h>

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: ./reverse [filename]\n");
        exit(EXIT_FAILURE);
    }
    char* filename = argv[1];
    FILE* fp = fopen(filename, "r");
    // int error_code = errno;
    // printf("Error Code: %d\n", error_code);
    if (fp == NULL) {
        fprintf(stderr, "fopen() error openning %s\n", filename);
        exit(EXIT_FAILURE);
    }

    // fseek() to the end of file and move back to find EOL
    int64_t offset = 1;
    int64_t offsetNextEOL = 0;
    int64_t offsetThisEOL = 0;
    // Find maximum offset
    if (fseek(fp, 0, SEEK_END) != 0) {
        fprintf(stderr, "fseek() error\n");
    }
    int64_t offsetMaximum = ftell(fp);
    // printf("Maximum offset: %ld\n", offsetMaximum);

    while (true) {
        if (fseek(fp, -offset, SEEK_END) != 0) {
            fprintf(stderr, "fseek() error\n");
            break;
        }
        else {
            char ch = fgetc(fp);
            if (ch == '\n') {
                // printf("Offset of EOL: %ld\n", offset);
                // print the last line to stdout
                offsetThisEOL = ftell(fp);
                if (offsetNextEOL < offsetThisEOL) {
                    // We just found the last EOL so print everything from it to EOF
                    if (fseek(fp, 0, SEEK_END) != 0) {
                        fprintf(stderr, "fseek() error\n");
                    }
                    int64_t offsetEOF = ftell(fp);
                    int64_t offsetDiff = offsetEOF - offsetThisEOL;
                    // Now seek back to offsetThisEOL and start print
                    if (fseek(fp, offsetThisEOL, SEEK_SET) != 0) {
                        fprintf(stderr, "fseek() error\n");
                    }
                    for (int64_t i = 0; i < offsetDiff; i++) {
                        printf("%c", fgetc(fp));
                    }
                    // For the last line, need to add EOL as it doesn't have one
                    printf("\n");
                    // We just found the last EOL so need to set up offsetNextEOL
                    offsetNextEOL = offsetThisEOL;
                }
                else {
                    // print everything from offsetThisEOL to offsetNextEOL
                    int64_t offsetDiff = offsetNextEOL - offsetThisEOL;
                    if (fseek(fp, offsetThisEOL, SEEK_SET) != 0) {
                        fprintf(stderr, "fseek() error\n");
                    }
                    for (int64_t i = 0; i < offsetDiff; i++) {
                        printf("%c", fgetc(fp));
                    }
                    offsetNextEOL = offsetThisEOL;
                }
                // Move offset 2 bytes from offsetThisEOL
                // If we only move 1 byte, a fgetc() still reads the EOL
                /* if (fseek(fp, offsetThisEOL - 2, SEEK_SET) != 0) {
                    fprintf(stderr, "fseek() error\n");
                } */
                // offset++;
            }
        }
        offset++;
        // >= doesn't work when first line is '\n'
        if (offset > offsetMaximum) {
            // We are still missing the first line because the beginning of the file doesn't contain EOL
            // print everything from beginning to offsetThisEOL (the first EOL)
            if (fseek(fp, 0, SEEK_SET) != 0) {
                fprintf(stderr, "fseek() error\n");
            }
            // Must use < offsetThisEOL - 1 to fit edge case where there is a '\n' line between two lines
            for (int64_t i = 0; i < offsetThisEOL - 1; i++) {
                printf("%c", fgetc(fp));
            }
            break;
        }
    }

    exit(EXIT_SUCCESS);
}