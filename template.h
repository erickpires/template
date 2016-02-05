#ifndef TEMPLATE_H
#define TEMPLATE_H 1


#define STUB_STR "???"

 typedef int bool;
 #define FALSE 0
 #define TRUE 1

typedef enum {
	dont_replace,
	replace_with_name,
	replace_with_argument
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

void exit_on_error(char*);
void eat_argument(int, char**, int, char**);

void trim_right(char*);
char* copy_string(char*, char*);
char* make_replace_str(char*);

int matches_file_format(char*, char*);
void fill_files_to_output_paths(int, char**, file_data*, char*,	char*);
void get_files_extensions(file_data*, int);
void get_files_names(file_data*, int, int);
DIR* get_template_dir(char*);
void get_template_files(file_data*, int, DIR*, char*);
bool file_exists(char*);
void copy_file(file_data*);

#endif
