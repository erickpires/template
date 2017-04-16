#ifndef TEMPLATE_H
#define TEMPLATE_H 1


#define STUB_STR "???"

#include <stdint.h>
typedef uint8_t uint8;

typedef int bool;
#define FALSE 0
#define TRUE 1

#define BUFFER_SIZE 1024

typedef enum {
    DONT_REPLACE,
    REPLACE_WITH_NAME,
    REPLACE_WITH_ARGUMENT
} replace_mode;

typedef struct {
    bool can_override;
    replace_mode replace;
    char* replace_string;
    char* file_path;
    char* file_extension;
    char* filename;
    char* template_file_path;
}file_data;

typedef struct dirent dir_ent;
typedef struct stat stat_buf;


void eat_argument(int, char**, int, char**);
void exit_on_error(char*, ...);
void trim_right(char*);
char* copy_string(char*, char*);
char* make_replace_str(char*);

int matches_file_format(char*, char*);
void fill_files_to_output_paths(int, char**, file_data*, char*,	char*);
void get_files_extensions(file_data*, int);
void get_files_names(file_data*, int);
DIR* get_template_dir(char*);
void get_template_files(file_data*, int, DIR*, char*);
bool file_exists(char*);
void copy_without_replacement(char*, char*);
void copy_file(file_data*);
bool is_absolute_path(char*);

#endif
