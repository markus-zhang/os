I have learned a lot so far since reading the source code of the 1991 version of `ls.c` of `coreutils` for Linux. I'll record everything I read **and will be used in my watered down version of `ls.c`** in this parchment.

1. Symbolic links: There are two of them, hard and soft. I don't know much about *hard*, but soft symbolic links are similar to Windows shortcuts. To create a symbolic link, use `ln`. Symbolic links show `l` when we run `ls -l`'
2. Names pipes: A FIFO, is a special file similar to a pipe but with a name on the filesystem. The show `p` when we tun `ls -l`. To create a names pipe, use `mkfifo`;
3. To determine the type of file, use `st_mode` in `struct stat` defined in `stat.h`.