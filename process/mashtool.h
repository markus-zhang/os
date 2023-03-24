#include <stdlib.h>
#include <string.h>

size_t count_char(char* haystack, char needle) {
    size_t len = strlen(haystack);
    size_t count = 0;
    for (int i = 0; i < len; i++) {
        if (needle == haystack[i]) {
            count++;
        }
    }
    return count;
}