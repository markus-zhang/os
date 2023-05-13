#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

ino_t get_inode(char* fname);

int main() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("Current directory name: %s\n", cwd);
    } else {
        perror("getcwd() error");
        return 1;
    }
    return 0;
}

ino_t get_inode(char* fname) {
    
}