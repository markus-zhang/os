### Project description

The OS project is my learning project using OSTEP (book & website, including assignments).

#### Shell

The `markus-shell`, `mash` is located in `./process`. It has the following features:
- Support 3 builtins: `exit`, `cd` and `path`
- Support redirection and parallel execution

Future .plan
- Add support for pipes (at the moment only support parsing pipes)
- Add support for combination of pipes and parallel execution:
    - e.g. `cmdA1 | ... | cmdAn & cmdB1 | ... | cmdBm`
- Add more builtins

#### ls

The `markus-ls`, `mls` is located in `./process`. It was developed from the original 1992 version of Linux `ls.c` and supports a few switches:
- `-l` for long format
- `-1` for one per line format
- `-F` for type indicator
- `-A` for all files
- `-a` for all files plus `.` and `..`
- `-Q` for quoted file names
- And a few others