#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define __STDC_WANT_LIB_EXT1__ 1
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
// #define _DEFAULT_SOURCE
#include <dirent.h>
#include <sys/stat.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include "mashtool.h"

#define COLOR_RED   "/033[0;31m]"
#ifndef S_IEXEC
#define S_IEXEC S_IXUSR
#endif

enum filetype {
    symbolic_link,
    named_pipe,
    directory,
    arg_directory,          /* Directory given as command line arg. Ended with forward slash '/' */
    normal			        /* All others. */
};

struct file {
    /* The file name */
    char* name;

    struct stat stat;

    /* For symbolic link, name of the file linked to, otherwise NULL */
    char* link_name;

    enum filetype filetype;

};

/* Do we trace links to pointed object or not? */

bool trace_links;

/* Array of files passed in argv */

struct file* files;

/* Length of block that `files' points to, measured in files. */
// This determines the allocated space of files//

int nfiles;

/* Index of first unused in `files'. */

int files_index;

/* Record of one pending directory waiting to be listed.  */

struct pending {
  char *name;
  /* If the directory is actually the file pointed to by a symbolic link we
     were told to list, `realname' will contain the name of the symbolic
     link, otherwise zero. */
  char *realname;
  struct pending *next;
};

/* Array of pending directories */

struct pending *pending_dirs;

/* If true then show directory name instead of contents */

bool immediate_dirs;

/* sort type to be used in different sorting options */

enum sort_type {
  sort_none,			/* -U */
  sort_name,			/* default */
  sort_extension,		/* -X */
  sort_time,			/* -t */
  sort_size			    /* -S */
};

enum sort_type sort_type;
bool sort_reverse;


enum format {
    many_per_line,       /* default */
    long_format,		/* -l */
    one_per_line,		/* -1 */
    with_commas			/* -m */
};

enum format format;


/* none means don't mention the type of files.
   all means mention the types of all files.
   not_programs means do so except for executables.
   Controlled by -F and -p.  */

enum indicator_style {
  none,				    /* default */
  all				    /* -F */
};

enum indicator_style indicator_style;


/* Nonzero means don't omit files whose names start with `.'.  -A */

bool all_files;

/* Nonzero means don't omit files `.' and `..'
   This flag implies `all_files'.  -a  */

bool really_all_files;

/* 
    Nonzero means we are listing the working directory because no
    non-option arguments were given. 
    Example: ls -aml
*/

bool dir_defaulted;

/* Print related global variables */

/* Nonzero means output each file name using C syntax for a string.
   Always accompanied by `quote_funny_chars'.
   This mode, together with -x or -C or -m,
   and without such frills as -F or -s,
   is guaranteed to make it possible for a program receiving
   the output to tell exactly what file names are present.  -Q  */

bool quote_as_string;


/* Do we mark funny chars as '?' */
bool qmark_funny_chars;

/* Total length of terminal line in char */
int line_length;

/* file permission */
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

/* -n switch: print user and group id instead of names */
bool numeric_users;

/* Nonzero means print each directory name before listing it. */

bool print_dir_name;

/* non-main functions declaration */
int32_t decode_switches(int argc, char** argv);
void usage();
void clear_files();
int gobble_file(char* name, bool explicit_arg, char* dirname);
void make_full_path(char* path, char* dirname, char* name);
void get_link_name(char* filename, struct file* f);
char* make_link_path(char* path, char* linkname);
void queue_directory (char* name, char* realname);
void extract_dirs_from_files (char* dirname, int recursive);
bool is_not_dot_or_dotdot(char* name);
void print_current_files();
void print_dir(char* name, char* realname);
int length_of_file_name_and_frills(struct file* f);
void print_type_indicator(unsigned int mode);
void print_file_name_and_frills(struct file* f);
void print_name_with_quoting(char* name);
void print_with_commas();
void print_long_format(struct file* f);
char* get_three_permission_chars(enum permission p);

/* Entry point of the program */

int 
main(int argc, char* argv[]) {
    int32_t i = 0;

    /* 
        Initialize global variables;
        Put all switches to default value;
    */

    files = NULL;
    pending_dirs = NULL;
    immediate_dirs = false;
    sort_type = sort_name;
    sort_reverse = false;
    indicator_style = none;
    all_files = false;
    really_all_files = false;
    dir_defaulted = true;
    trace_links = false;
    quote_as_string = false;
    qmark_funny_chars = true;
    line_length = 80;   // default on 80 characters per line
    numeric_users = false;
    print_dir_name = true;

    #ifdef TIOCGWINSZ
    {
        struct winsize ws;

        if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != -1 && ws.ws_col != 0) {
            line_length = ws.ws_col;
        }
    }
    #endif

    /* Modify switches based on user input */

    i = decode_switches(argc, argv);

    // ------- DEBUG
    // printf("i is %d, argc is %d\n", i, argc);

    /* Initialize array of file */
    nfiles = 0x100;
    files = malloc(nfiles * sizeof(struct file));
    files_index = 0;

    clear_files();

    
    /*
        Now we have option part of argv,
        if we have more contents they mean files/directories/whatever,
        or if we don't have anything else it means to print the cwd.

        Example: ls -ml
        - Two options and done, this means to print the cwd with 'm' 'l' switches

        Example: ls -R ~/Develop
        - One option, and then a folder (or a file as we are not sure which one it is)
    */

    if (i < argc) {
        // We do have some non-option args
        dir_defaulted = false;
    }

    for (; i < argc; i++) {
        // explicit_arg set to true as user explicitly gives args
        gobble_file(argv[i], true, "");
    }

    // when i == argc, dir_defaulted = true, we are basically showing .
    // if immediate_dirs is true, we just show ., not the contents (pretty dumb TBH)

    if (dir_defaulted) {
        if (immediate_dirs) {
            /*
                We might want to understand what happens in gobble_file(".", 1)
                Best way is to first look at the original ls.c, which is gobble_file(".", 1, "")
                - Initiate files[files_index].linkname and linkmode (not available in my version)
                - Scroll down until S_ISDIR block because it is a directory
                - Recall that immediate_dirs is true, so files[files_index].filetype = directory
                This basically says, OK, "." is a directory and we should only print itself

                Now if we scroll further down to print_current_files()
                let's pick case many_per_line and scroll to print_many_per_line()
                Following function call, it goes to print_file_name_and_frills()


            */
        
            gobble_file(".", 1, "");
        }
        else {
            queue_directory(".", 0);
        }
    }

    // If there are more than 0 file grobbed
    if (files_index) {
        // ----------DEBUG
        printf("files_index: %d\n", files_index);
        /* Let's assume we don't sort */
        // sort_files();
        if (!immediate_dirs) {
            extract_dirs_from_files("", 0);
        }
    }

    /* extract_dirs_from_files() may decrease files_index so check again */
    if (files_index) {
        // ----------DEBUG
        printf("files_index: %d\n", files_index);

        print_current_files();
    }

    struct pending* walker = NULL;
    while(pending_dirs) {
        printf("Pending DIRs: %s\n", pending_dirs->name);
        walker = pending_dirs;
        pending_dirs = pending_dirs->next;
        print_dir(walker->name, walker->realname);
        free(walker->name);
        if (walker->realname) {
            free(walker->realname);
        }
        free(walker);
        // why? I don't understand, just copied
        // print_dir_name = true;
    }

    return EXIT_SUCCESS;
}

/* long option struct for decode_switches() */
struct option long_options[] =
{
    {"all", 0, 0, 'a'},
    {"escape", 0, 0, 'b'},
    {"numeric-uid-gid", 0, 0, 'n'},
    {"reverse", 0, 0, 'r'},
    {"width", 1, 0, 'w'},
    {"almost-all", 0, 0, 'A'},
    {"file-type", 0, 0, 'F'},
    {"dereference", 0, 0, 'L'},
    {"literal", 0, 0, 'N'},
    {"quote-name", 0, 0, 'Q'},
    {"recursive", 0, 0, 'R'},
    {0, 0, 0, 0}
};

int32_t 
decode_switches(int argc, char** argv) {
    /*
        Reading each argv and modify switches
    */
    int c;
    /* set default value of format to many_per_line */
    format = many_per_line;
    
    while ((c = getopt_long(argc, argv, "abdlmnqrtwAFLSUX1", long_options, NULL)) != EOF) {
        switch (c) {
            case 'a':
                all_files = true;
                really_all_files = true;
                break;
            case 'b':
                qmark_funny_chars = false;
                break;
            case 'd':
                immediate_dirs = true;
            case 'l':
                format = long_format;
                break;
            case 'm':
                format = with_commas;
                break;
            case 'n':
                numeric_users = true;
                break;
            case 'q':
                qmark_funny_chars = true;
                break;
            case 'r':
                sort_reverse = true;
                break;
            case 't':
                sort_type = sort_time;
                break;
            case 'w':
                line_length = atoi(optarg);
                if (line_length < 16) {
                    fprintf(stderr, "line_length must be greater than or equal to 16\n");
                }
                break;
            case 'A':
                all_files = true;
                break;
            case 'F':
                indicator_style = all;
                break;
            case 'L':
                trace_links = true;
                break;
            case 'S':
                sort_type = sort_size;
                break;
            case 'U':
                sort_type = sort_none;
                break;
            case 'X':
                sort_type = sort_extension;
                break;
            case '1':
                format = one_per_line;
                break;
            default:
                // Wrong argument, print usage
                usage();
                break;
        }
    }

    /* Index in ARGV of the next element to be scanned.
    This is used for communication to and from the caller
    and for communication between successive calls to 'getopt'.

    On entry to 'getopt', zero means this is the first call; initialize.

    When 'getopt' returns -1, this is the index of the first of the
    non-option elements that the caller should itself scan.

    Otherwise, 'optind' communicates from one call to the next
    how much of ARGV has been scanned so far.  */
    return optind;
}

void 
usage() {
    printf("ls: invalid option\n");
    printf("Try 'ls --help' for more information.\n");
}

void 
clear_files() {
    for (int i = 0; i < nfiles; i++) {
        if (files[i].name) {
            free(files[i].name);
        }
        files[i].name = NULL;
        if (files[i].link_name) {
            free(files[i].link_name);
        }
        files[i].link_name = NULL;
    }
    files_index = 0;
}

int 
gobble_file(char* name, bool explicit_arg, char* dirname) {
    // gobble_file runs inside a loop and each loop consumes a new filename.
    // Potentially the # of files can reach files_index,
    // and we will have to realloc the array.
    // We follow the simple strategy of doubling the space of the array
    // everytime the array reaches nfiles

    char* path = NULL;

    if (files_index == nfiles) {
        nfiles *= 2;
        files = realloc(files, nfiles * sizeof(struct file));
    }

    if (explicit_arg) {
        if (dirname[0] == 0) {
            // Don't have a directory name
            path = name;
        }
        else {
            path = malloc(strlen(name) + strlen(dirname) + 2);
            make_full_path(path, dirname, name);
        }
    }

    int errno = stat(path, &(files[files_index].stat));
    if (errno == -1) {
        // FIXME: OK for . and .., but immediately failed for loop19
        // Argument: /dev
        // TODO: OK I got it, loop19 doesn't exist, should be /dev/loop19
        perror("stat failed");
        exit(EXIT_FAILURE);
    }

    /* if trace symbolic links, use stat(), else use lstat() */
    // Please note that we don't actually know whether the name is a symbolic link
    // Neither do we know whether the pointed object is valid even if it is
    // Thus we catch exception here to use lstat() if stat() fails
    // The purpose of the next block is just to load struct stat
    files[files_index].link_name = NULL;
    files[files_index].name = NULL;

    if (trace_links) {
        // We first use stat() and assume the link is valid
        if (stat(name, &(files[files_index].stat)) != 0) {
            // Then if it fails we switch to lstat()
            if (lstat(name, &(files[files_index].stat)) != 0) {
                fprintf(stderr, "lstat() failed in %s\n", __func__);
            }
        }
    }
    else {
        // Since we don't care about pointed object, just use lstat()
        if (lstat(name, &(files[files_index].stat)) != 0) {
            fprintf(stderr, "lstat() failed in %s\n", __func__);
        }
    }

    // Now struct stat has been loaded, we start to process information for symbolic links
    // S_ISLNK() returns true if file is
    #ifdef S_ISLNK
    if (S_ISLNK(files[files_index].stat.st_mode)) {
        get_link_name(name, &files[files_index]);
        // Let's ignore make_link_path first
        make_link_path(NULL, NULL);
        files[files_index].filetype = symbolic_link;
    }
    #endif

    // Now we turn to the case of directories
    // I think filetype leads to different print outs, will see
    #ifdef S_ISDIR
    if (S_ISDIR(files[files_index].stat.st_mode)) {
        if (explicit_arg && !immediate_dirs) {
            files[files_index].filetype = arg_directory;
        }
        else {
            files[files_index].filetype = directory;
        }
    }
    #endif

    // Now we tuen to blocks
    // We don't implement --block-size so I'll just use stat.st_block
    // which is default to block size of 512B
    int blocks = files[files_index].stat.st_blocks;
    files[files_index].name = strndup(name, strlen(name) + 1);
    // debug
    // printf("gobble_file(): file name: %s\n", files[files_index].name);
    files_index++;

    return blocks;
}

void
make_full_path(char* path, char* dirname, char* name) {

    /* No need to copy dirname if dirname is '.' or empty */
    if (dirname[0] == '.' && dirname[1] == 0) {
        path = name;
    }
    else {
        while (*dirname) {
            *path++ = *dirname++;
        }
        /* Do we need an extra '/' between dirname and name? */
        if (dirname[-1] != '/') {
            *path++ = '/';
        }
        while (*name) {
            *path++ = *name++;
        }
        *path = '\0';
    }
    printf("Full path: %s\n", path);
}

void 
get_link_name(char* filename, struct file* f) {
    /*
        For symbolic links, use readlink() to fetch information of linked object
        We need to be careful about broken symbolic links though
        Those links don't link to anything
    */
    int buffer_size = f->stat.st_size;
    // Add 1 for '\0' just in case (usually linkbuffer only needs like 100 char maximum)
    char* linkbuffer = malloc(buffer_size + 1);
    int len;
    if ((len = readlink(filename, linkbuffer, buffer_size)) != -1) {
        linkbuffer[len] = '\0';
        f->link_name = linkbuffer;
    }
    else {
        fprintf(stderr, "readlink() error: %s\n", __func__);
        free(linkbuffer);
        linkbuffer = NULL;
        exit(EXIT_FAILURE);
    }
}

void
queue_directory (char* name, char* realname) {
    struct pending* new = malloc(sizeof(struct pending));
    // Looks like it's prepending struct pending*
    // First node is: node1->node1
    // When the second gets inserted, we have node2->next = node1 and pending_dirs = node2
    new->next = pending_dirs;
    pending_dirs = new;
    new->name = strndup(name, strlen(name) + 1);

    if (realname) {
        new->realname = strndup(realname, strlen(realname) + 1);
    }
    else {
        new->realname = NULL;
    }
}

/*  If `linkname' is a relative path and `path' contains one or more
    leading directories, return `linkname' with those directories
    prepended; otherwise, return a copy of `linkname'.
    If `linkname' is zero, return zero. 
*/

/*  Relative path is defined as path related to the present working directory(pwd). 
    Consider there is a path: ~/Develop/sysprog/os/process
    We can go down to ~/Develop/sysprog/os/process, and type:
    ln -s ../os/process ~/process
    This will create a symbolic link to a relactive path 
*/

/*
    An absolute path always starts with '/'
    /home/Develop/sysprog/os/process
*/

char* 
make_link_path(char* path, char* linkname) {
    return NULL;
}


/* Remove any entries from `files' that are for directories,
   and queue them to be listed as directories instead.
   `dirname' is the prefix to prepend to each dirname
   to make it correct relative to ls's working dir.
   `recursive' is nonzero if we should not treat `.' and `..' as dirs.
   This is desirable when processing directories recursively.  */

void
extract_dirs_from_files (char* dirname, int recursive) {
    // int dirlen = strlen(dirname) + 2;

    /* Queue the directories last one first, because queueing reverses the
     order.  */

    for (int i = files_index - 1; i >= 0; i--) {
        if ((files[i].filetype == directory || files[i].filetype == arg_directory)
            && (!recursive || is_not_dot_or_dotdot(files[i].name))) {
            if (files[i].name[0] == '/' || dirname[0] == 0) {
                queue_directory(files[i].name, files[i].link_name);
            }
            else {
                // No idea what this part does so ignore first
            }
            if (files[i].filetype == arg_directory) {
                free(files[i].name);
            }
        }
    }

    /* Now delete the directories from the table, compacting all the remaining
     entries.  */
    int i = 0;
    int j = 0;
    
    for (; i < files_index; i++) {
        if (files[i].filetype != arg_directory) {
            files[j++] = files[i];
        }
    }
    files_index = j;
}

bool
is_not_dot_or_dotdot(char* name) {
    if ((strlen(name) == 1) && (strncmp(name, ".", 1) == 0)) {
        return true;
    }
    if ((strlen(name) == 2) && (strncmp(name, "..", 2) == 0)) {
        return true;
    }
    return false;
}

void
print_current_files() {
    switch(format) {
        case one_per_line:
            for (int i = 0; i < files_index; i++) {
                /* Directly using pointer arithmetric */
                print_file_name_and_frills(files + i);
                putchar('\n');
            }
            break;
        case with_commas:
            print_with_commas();
            break;
        /* long format is the -l switch, thus one file per line */
        case long_format:
            for (int i = 0; i < files_index; i++) {
                print_long_format(&files[i]);
                putchar('\n');
            }
            break;
        case many_per_line:
            // TODO: implement this properly
            print_with_commas();
            break;
    }
}

void 
print_file_name_and_frills(struct file* f) {
    // ignored inode, block size

    print_name_with_quoting(f->name);

    /* print file type indicator (e.g. directory get '/') */
    if (indicator_style != none) {
        print_type_indicator(f->stat.st_mode);
    }
}

void
print_name_with_quoting(char* name) {
    if (quote_as_string) {
        putchar('"');
    }

    for (int i = 0; i < strlen(name); i++) {
        char c = name[i];
        // ignore quote_funny_characters for now
        if (c >= 0x20 && c <= 0x7E) {
            putchar(c);
        }
        else if (!qmark_funny_chars) {
            putchar(c);
        }
        else {
            putchar('?');
        }
    }

    if (quote_as_string) {
        putchar('"');
    }
}

/* prints indicator immediately following file name */

void
print_type_indicator(unsigned int mode) {
    if (S_ISDIR(mode)) {
        putchar('/');
    }

    if (S_ISLNK(mode)) {
        putchar('@');
    }

    if (S_ISFIFO(mode)) {
        putchar('|');
    }

    if (S_ISSOCK(mode)) {
        putchar('=');
    }

    /*
        Here's how the expression evaluates step by step:

        S_IEXEC is a bitmask that represents the execute permission bit, 
        which has a value of 0100 in octal notation or 0x40 in hexadecimal notation.

        S_IEXEC >> 3 shifts the S_IEXEC bitmask to the right by 3 bits, 
        which results in a value of 0010 in octal notation or 0x08 in hexadecimal notation. 
        This represents the execute permission bit for the group.

        S_IEXEC >> 6 shifts the S_IEXEC bitmask to the right by 6 bits, 
        which results in a value of 0001 in octal notation or 0x01 in hexadecimal notation. 
        This represents the execute permission bit for others.

        The three values are combined using the bitwise OR operator (|), 
        which results in a bitmask with the executable permission bit set in 
        the least significant three bits, the middle three bits, and the most significant three bits.
        The resulting value is 0111 in octal notation or 0x49 in hexadecimal notation.

        The resulting bitmask can be used to test if any of the permission bits have the 
        executable permission enabled. 
        
        For example, (mode & (S_IEXEC | S_IEXEC >> 3 | S_IEXEC >> 6)) can be used to test 
        if the file represented by the mode bitmask has executable permission 
        for the owner, group, or others.
    */
    if (S_ISREG(mode) && indicator_style == all) {
        if (mode & (S_IEXEC | S_IEXEC >> 3 | S_IEXEC >> 6)) {
            putchar('*');
        }
    }
}

void
print_with_commas() {
    /*
        Description of original 1991 ls.c algorithm:

        First calculate the # of chars needed to display file N;
        Then look forward to find out if there is another file to display;
        If there is one, then we need to add a comma and a space;
        Whence everything has been calculated, we get how long the whole string is;
        We then look at the current cursor position and check we have enough space for the line;
        If not then we print a '\n';
    */
    
    int cur = 0;
    int cur_next = 0;

    for (int i = 0; i < files_index; i++) {
        cur_next += length_of_file_name_and_frills(files + i);
        if (i + 1 < files_index) {
            cur_next += 2;
        }
        if (cur_next > line_length && i > 0) {
            putchar('\n');
            cur_next -= cur;
        }
        if (i + 1 < files_index) {
            printf(", ");
        }
    }
}

void 
print_long_format(struct file* f) {
    /*
        drwxr-xr-x  2 root    root         540 Mar 31 15:19 block

        d: file type
        rwxr-xr-x: permission
    */

    /* file type, 1 character */
    char type;
    if (S_ISDIR(f->stat.st_mode)) {
        type = 'd';
    }
    else if (S_ISLNK(f->stat.st_mode)) {
        type = 'l';
    }
    else if (S_ISCHR(f->stat.st_mode)) {
        // Character devices are files that provide access to a device that
        // transfers data one character at a time, such as keyboard or a serial port
        type = 'c';
    }
    else if (S_ISBLK(f->stat.st_mode)) {
        type = 'b';
    }
    else if (S_ISFIFO(f->stat.st_mode)) {
        type = 'p';
    }
    #ifdef S_ISSOCK
    else if (S_ISSOCK(f->stat.st_mode)) {
        type = 's';
    }
    #endif
    // regular file
    else {
        type = '-';
    }
    putchar(type);

    /* 
        permissions, 9 characters 
        3 chars for the owner, 3 chars for the group that owns the file,
        3 chars for all other users
        for the 3 characters:
        char 1: whether can read
        char 2: whether can write
        char 3: whether can execute
    */

    int permission_owner = (f->stat.st_mode >> 6) & 07;
    int permission_group = (f->stat.st_mode >> 3) & 07;
    int permission_other = (f->stat.st_mode >> 0) & 07;

    char* ownerp = get_three_permission_chars(permission_owner);
    char* groupp = get_three_permission_chars(permission_group);
    char* otherp = get_three_permission_chars(permission_other);

    printf("%s", ownerp);
    printf("%s", groupp);
    printf("%s", otherp);
    putchar(' ');
    putchar(' ');

    /* Number of hard links, from st_nlink */
    printf("%ld ", f->stat.st_nlink);

    /* 
        Owner ID or Owner name 
        Owner ID can be grabbed from st_uid
        With that info we can pass it to getpwuid to get a struct passwd*,
        which contains username:

        struct passwd {
            char   *pw_name;       // username
            char   *pw_passwd;     // user password
            uid_t   pw_uid;        // user ID
            gid_t   pw_gid;        // group ID 
            char   *pw_gecos;      // user information 
            char   *pw_dir;        // home directory 
            char   *pw_shell;      // shell program 
        };
    */

    int owner_uid = f->stat.st_uid;
    if (numeric_users) {
        printf("%d\t\t", owner_uid);
    }
    else {
        struct passwd* pwd = getpwuid(owner_uid);
        printf("%s\t", pwd->pw_name);
    }

    /* 
        For group name, user getgrgid(), which returns a struct group:
        struct group {
            char   *gr_name;        // group name
            char   *gr_passwd;      // group password
            gid_t   gr_gid;         // group ID
            char  **gr_mem;         // NULL-terminated array of pointers to names of group members
        };
    */

    int group_uid = f->stat.st_gid;
    if (numeric_users) {
        printf("%d\t\t", group_uid);
    }
    else {
        struct group* grp = getgrgid(group_uid);
        printf("%s\t\t", grp->gr_name);
    }

    /*
        File size, can be grabbed from st_blocks
        But for directories and symbolic links,
        this shows the size of the data associated with the directory or link
    */
    
    // TODO: Implement code for directories and symbolic links
    int size = f->stat.st_size;
    printf("%d ", size);

    /* Modification time */

    char time_human_readable[64];
    time_t last_modified_time = f->stat.st_mtime;
    struct tm* temp;
    temp = localtime(&last_modified_time);
    if (temp == NULL) {
        fprintf(stderr, "localtime() failed: %s\n", __func__);
        exit(EXIT_FAILURE);
    }
    if (strftime(time_human_readable, sizeof(time_human_readable), "%M %d %H%M", temp) == -1) {
        fprintf(stderr, "strftime() failed: %s\n", __func__);
        exit(EXIT_FAILURE);
    }
    printf("%s ", time_human_readable);

    /* filename, for symbolic links print its link_name. Print file indicator char if needed */
    printf("%s", f->name);

    if (f->filetype == symbolic_link) {
        if (f->link_name) {
            printf(" -> ");
            print_name_with_quoting(f->link_name);
            if (indicator_style != none) {
                // TODO: Check original code and maybe implement linkmode
                print_type_indicator(f->stat.st_mode);
            }
        }
    }
    else if (indicator_style != none) {
        print_type_indicator(f->stat.st_mode);
    }
}

char* 
get_three_permission_chars(enum permission p) {
    switch(p) {
        case no_permission:
            return "---";
        case can_exec:
            return "--x";
        case can_write:
            return "-w-";
        case can_exec_write:
            return "-wx";
        case can_read:
            return "r--";
        case can_exec_read:
            return "r-x";
        case can_read_write:
            return "rw-";
        case all_permission:
            return "rwx";
        default:
            return "---";
    }
}

int 
length_of_file_name_and_frills(struct file* f) {
    int len = 0;

    if (quote_as_string) {
        // Add space for two quotes
        len += 2;
    }

    len += strlen(f->name);

    // Show indicator for file types
    if (indicator_style != none) {
        len++;
    }

    return len;
}

/*
    This is the meat: printing all files under the directory
    The naming of the function is a bit misleading
    as I thought it has seomthing to do with directories, not files
*/
void 
print_dir(char* name, char* realname) {
    DIR* reading;
    struct dirent* next;

    if ((reading = opendir(name)) == NULL) {
        fprintf(stderr, "opendir() failed: %s\n", __func__);
        exit(EXIT_FAILURE);
    }

    // clear_files();

    while ((next = readdir(reading)) != NULL) {
        gobble_file(next->d_name, 0, name);
    }

    printf("%s: gobble_file() completed: %d files_index\n", __func__, files_index);

    if (closedir(reading)) {
        fprintf(stderr, "closedir() failed: %s\n", __func__);
        exit(EXIT_FAILURE);
    }

    // TODO: implement sort_files()
    // sort_files()

    if (print_dir_name) {
        if (realname) {
            printf("%s:\n", realname);
        }
        else {
            printf("%s:\n", name);
        }
    }

    if (files_index) {
        print_current_files();
    }

    if (pending_dirs) {
        putchar('\n');
    }

}
