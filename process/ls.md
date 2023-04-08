I have learned a lot so far since reading the source code of the 1991 version of `ls.c` of `coreutils` for Linux. I'll record everything I read **and will be used in my watered down version of `ls.c`** in this parchment.

1. Symbolic links: There are two of them, hard and soft. I don't know much about *hard*, but soft symbolic links are similar to Windows shortcuts. To create a symbolic link, use `ln`. Symbolic links show `l` when we run `ls -l`'
2. Names pipes: A FIFO, is a special file similar to a pipe but with a name on the filesystem. The show `p` when we tun `ls -l`. To create a names pipe, use `mkfifo`;
3. To determine the type of file, use `st_mode` in `struct stat` defined in `stat.h`.
4. `st_mode` can be used to determine the 9 permission characters:
    ```c
    enum permission {
        no_permission =         0,
        can_exec =              1,
        can_write =             2,
        can_exec_write =        3,
        can_read =              4,
        can_exec_read =         5,
        can_read_write =        6,
        all_permission =        7
    };

    int permission_owner = (f->stat.st_mode >> 6) & 07;
    int permission_group = (f->stat.st_mode >> 3) & 07;
    int permission_group = (f->stat.st_mode >> 0) & 07;
    ```
5. This is more general C programming: Why does passing pointer to literal string out of scope work?
    ```c
    char* pass_literal_string() {
        char* temp = "abcdefg";
        return temp;
    }

    int main() {
        char* blah = pass_literal_string();
        printf("%s\n", blah);

        return 0;
    }
    ```
    Why does the above code work while "abcdefg" is in another scope? Because they are usually put in **read-only-data** section, not on stack.

    This, instead, does NOT work:
    ```c
    int* pass_integer() {
        int temp = "abcdefg";
        return &temp;
    }

    int main() {
        int* blah = pass_integer();
        printf("%d\n", blah);

        return 0;
    }
    ```
6. How to convert a long epoch to human readable time in C?
   https://stackoverflow.com/questions/64725951/convert-long-of-unix-epoch-timestamp-to-real-date-time


### Program structure

#### Switches

1. Format: should only support short format (default: multiple in one line), one per line (`-1`) and long format (`-l`). Added: also support with_commas (`-m`)
   1. For short format, we need to consider all other format modifiers before writing the whole line. For example, what if user enables indicator? what if user wants quotation marks for each file? 
2. Indicator: Does the program show extra char for each file? (`-F`)
3. `-A` for including fies starting with `.` and `-a` to further include `.` and `..`
4. `-Q` to put `"` between file names
5. For long format, if print user id and group id instead of names, use `-n`
6. For directories, `-d` (immediate_dirs) prints info about the directory ONLY instead of going into the directory and print everything inside
7. For symbolic links, `-L` shows info of the object pointed by the symblic link

#### Pseudo code

We can probably re-use a lot of the code and even patterns in the original experiment project.

```c
int main(int argc, char* argv[]) {
    // set default values for all switches

    // decode switches to set them to proper values based on argv
    // it's smart to return total argv elements read (i)

    // initialize files

    // use i to figure out whether we have parsed all argv elements

    /*
        if we have parsed all argv elements: {
            // then we are printing the current work directory . 
            if we only print directory name: {
                gobble cwd only
            }
            else {
                gobble all files under cwd, including . and ..
            }
        }
        else {
            for (; i < argc; i++) {
                switch (file type) {
                    case (regular file):
                        gobble file
                        break
                    case (directory):
                        if (only print dir name):
                            gobble dir only
                        else:
                            gobble all files under dir, including . and ..
                        break;
                    case (symbolic link):
                        if (show pointed object):
                            if (pointed object is dir):
                                if (only print dir name):
                                    gobble dir only
                                else:
                                    gobble all files under dir, including . and ..
                            else:
                                gobble pointed object
                        else:
                            gobble file
                        break;
                    default:
                        break;
                }
            }   
        }

        for each file in files:
            print_file(file)
    */

    /*
        print_file(file) {
            
        }
    */

    
}
```
