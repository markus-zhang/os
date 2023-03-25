Some notes on processes:

- child inherits `stdout`, `stdin` and `stderr` from parent code *before* `fork()`;
- child can close `stdout` but if parent does not close it, when the child processes returns, `stdout` is still open to parent;

`gcc -std=c11 -D_DEFAULT_SOURCE -Wall -Werror -pedantic ls_old.c -o ls_old` 