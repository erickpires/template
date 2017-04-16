/* This program is a C implementation of the template script
 * written by Ian Brunelli.
 *
 * Usage:
 *
 * A destination directory can be specified with -d [DIR], otherwise, the
 * current dir will be used.
 * If the destination file already exists, the program aborts with an error
 * message, unless -o is passed as argument.
 * If the template files have the string "???" and -r [STR] is passed as argument
 * the "???" is replaced with STR. If -r is the last argument and STR is omitted,
 * the default behavior is to replace "???" with the filename in uppercase
 * with all its non-alphanumeric characters replaced with '_' (for use with C header files).

 * Erick Pires - 25/12/14
 */

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>

#include "template.h"
#include "string_buffer.h"

#define starts_with(str,ch) (*str == ch)

void exit_on_error(char* msg){
    fprintf(stderr, msg);
    exit(-1);
}

// TODO: Error messages are terrible. Use something as var_args to improve it.
void eat_argument(int argc, char** argv, int index, char** dest) {
    if(index >= argc) {
        exit_on_error("Too few arguments.\n");
    }

    *dest = argv[index];
    argv[index] = NULL;
}

inline void trim_right(char* str){
    int len = strlen(str);

    int i  = len - 1;

    while( i >= 0 &&
           ( str[i] == '\n' ||
             str[i] == '\t' ||
             str[i] == ' ')) {
        str[i--] = '\0';
    }
}

inline int matches_file_format(char* filename, char* file_extension) {
    int end_filename = strlen(filename);
    int end_file_extension = strlen(file_extension);

    int index_filename = end_filename - 1;
    int index_file_extension = end_file_extension - 1;

    // NOTE(erick): The filename has to be bigger than the extension by
    // at least two characters (one of them been the dot)
    if(index_filename < index_file_extension + 2) {
        return 0;
    }

    // NOTE(erick): We do not support template files without extension
    if(index_file_extension < 0) {
        return 0;
    }

    while(filename[index_filename] == file_extension[index_file_extension]) {
        // NOTE(erick): When the extension ends, we check if we are on a dot ('.')
        // in the filename in which case we return true.
        if(index_file_extension == 0) {
            if(filename[index_filename - 1] == '.') {
                return 1;
            } else {
                return 0;
            }
        }

        index_filename--;
        index_file_extension--;
    }

    return 0;
}

inline char* copy_string(char* dest, char* src){
    //NOTE: copies all the characters from src to dest and
    //returns a pointer to last character of dest.

    while(*src) {
        *dest++ = *src++;
    }

    *dest++ = '\0';
    return dest;
}

void fill_files_to_output_paths(int argc, char** argv,
                                file_data* files_to_output,
                                char* destination_dir,
                                char* filenames_memory){

    for(int arg_index = 1; arg_index < argc; arg_index++) {
        char* current_argument = argv[arg_index];
        if(current_argument == NULL) // This argument has already been eaten
            continue;

        if(starts_with(current_argument, '-')) { // It's an option
            current_argument++;
            switch(*current_argument) {
            case 'o' :
                files_to_output->can_override = TRUE;
                break;
            case 'r' :
                files_to_output->replace = REPLACE_WITH_NAME;
                break;
            case 'e' :
                arg_index++;
                eat_argument(argc, argv, arg_index, &(files_to_output->file_extension));
                break;
            case 'R' :
                files_to_output->replace = REPLACE_WITH_ARGUMENT;
                arg_index++;
                eat_argument(argc, argv, arg_index, &(files_to_output->replace_string));
                break;
            case 'd' :
                break;
            default :
                exit_on_error("Unknown option.");
            }
            // We processed the option and can advance the loop.
            continue;
        }

        // Setting the point to the destination

        files_to_output->file_path = filenames_memory;

        char* current_file_path = current_argument;
        if(!is_absolute_path(current_file_path)) {
            filenames_memory =  copy_string(filenames_memory, destination_dir);
            // We return one character to point the '\0' of the last string.
            filenames_memory--;
            filenames_memory = copy_string(filenames_memory, "/");
            filenames_memory--;
        }

        // NOTE(erick): If the path is absolute we don't prepend the destination_dir path.
        // In fact, we could just point the files_to_output->file_path to current_file_path
        // (without a copy), but making the copy keeps the code more symmetric and less
        // error prone (someone could write to the argv strings).
        filenames_memory = copy_string(filenames_memory, current_argument);
        files_to_output++;
    }
}

void get_files_extensions(file_data* files_to_output, int files_to_output_count) {
    for(int i = 0; i < files_to_output_count; i++){
        if(files_to_output[i].file_extension == NULL) {
            int pos = strlen(files_to_output[i].file_path) - 1;
            while(files_to_output[i].file_path[pos] != '.' && pos > 0){ //TODO: extremely confusing
                pos--;
            }
            if(pos == 0) {
                exit_on_error("You must specify an extension.\n");
            }

            files_to_output[i].file_extension = files_to_output[i].file_path + pos + 1;
        }
    }
}

void get_files_names(file_data* files, int files_count) {
    // NOTE(erick): We now support absolute paths, so we can't count on
    // the destination directory been always the same (as we did before).
    // We have to iterate backwards in the file_path until we find a '/'.
    // Note the a slash will always be found since an absolute path starts
    // with a slash and the relative paths are concatenated with a the
    // destination directory and a '/'.
    for(int file_index = 0; file_index < files_count; file_index++) {
        file_data* current_file = files + file_index;
        int file_path_len = strlen(current_file->file_path);

        if(file_path_len == 0) {
            exit_on_error("File path can't be empty");
        }
        // Loop until a '/' is found or cursor reaches the beginning of
        // the filename. This second condition should not happen as stated
        // above.
        char* cursor;
        for(cursor = current_file->file_path + (file_path_len - 1);
            *cursor != '/' && cursor > current_file->file_path;
            cursor--) { ; }

        // 'cursor + 1' is always valid since we subtract 1 from the length.
        current_file->filename = cursor + 1;
    }
}

DIR* get_template_dir(char* template_dir_name_buffer) {

    FILE* xdg_result = NULL;
    //NOTE: Since we only have access to the stdin stream
    //      The error stream is redirected so we can read it.
    xdg_result = popen("xdg-user-dir TEMPLATES 2>&1", "r");

    if(!xdg_result) {
        exit_on_error("Failed to use popen\n");
    }

    fgets(template_dir_name_buffer, 256, xdg_result);

    pclose(xdg_result);

    if(strstr(template_dir_name_buffer, "sh:")) {
        exit_on_error("Could not execute xdg-user-dir\n");
    }

    trim_right(template_dir_name_buffer); // Remove the last new line

    return opendir(template_dir_name_buffer);
}

void get_template_files(file_data* files, int files_count,
                        DIR* template_dir, char* template_dir_name){

    int completed_files = 0;
    dir_ent* template_file_dir_ent;

    while((template_file_dir_ent = readdir(template_dir))) {
        char* template_file_full_path = NULL;
        char* current_template_filename = template_file_dir_ent->d_name;

        // NOTE(erick): Ignore dir_ent '.' and '..'
        if(strcmp(current_template_filename, ".") == 0 ||
           strcmp(current_template_filename, "..") == 0) {
            continue;
        }

        for(int i = 0; i < files_count; i++){
            if(matches_file_format(current_template_filename,
                                   files[i].file_extension)) {

                if(template_file_full_path == NULL) {

                    template_file_full_path = (char*) malloc(
                        strlen(template_dir_name) +
                        strlen(current_template_filename) +
                        strlen("/") + 1);

                    if(!template_file_full_path) {
                        exit_on_error("Could not allocate memory");
                    }

                    strcpy(template_file_full_path, template_dir_name);
                    strcat(template_file_full_path, "/");
                    strcat(template_file_full_path, current_template_filename);
                }

                files[i].template_file_path = template_file_full_path;
                completed_files++;

                if(completed_files == files_count) {
                    return;
                }
            }
        }
    }

    if(files_count != completed_files) {
        fprintf(stderr, "Error:\n");

        for(int i = 0; i < files_count; i++) {
            if(files[i].template_file_path == NULL) {
                fprintf(stderr, "\t\"%s\"\n", files[i].filename);
            }
        }

        exit_on_error("Could not find matching templates for the files above");
    }
}

char* make_replace_str(char* filename){
    char* result = (char*) calloc(strlen(filename) + 1, sizeof(char));
    if(!result) {
        exit_on_error("Could not allocate memory");
    }

    for(int i = 0; i < strlen(filename); i++) {
        if(isalpha(filename[i])) {
            result[i] = filename[i] & (~0x20);
        } else {
            result[i] = '_';
        }
    }
    return result;
}

inline bool file_exists(char* file_path) {
    return access(file_path, F_OK) == 0;
}

void copy_without_replacement(char* template_file_path, char* output_file_path) {
    FILE* template_file = fopen(template_file_path, "rb");
    //TODO(erick): modify exit_on_error to use va
    if(!template_file) {
        fprintf(stderr, "Could not open the file: \"%s\"\n", template_file_path);
        exit(-1);
    }

    FILE* output_file = fopen(output_file_path, "wb");

    uint8 buffer[BUFFER_SIZE];
    //TODO(erick): modify exit_on_error to use va
    if(!output_file) {
        fprintf(stderr, "Could not open the file: \"%s\"\n", output_file_path);
        exit(-1);
    }

    while(TRUE) {
        size_t n_read = fread(buffer, sizeof(uint8), BUFFER_SIZE, template_file);
        if(n_read == 0) {
            break;
        }

        size_t n_written = fwrite(buffer, sizeof(uint8), n_read, output_file);

        if(n_written != n_read) {
            fprintf(stderr, "Failed to write the complete input to the output file: \"%s\"\n", output_file_path);
            exit(-1);
        }
    }

    fclose(template_file);
    fclose(output_file);
}

void copy_file(file_data* file) {
    stat_buf stat_buffer;
    char* template_file_path = file->template_file_path;
    char* output_file_path = file->file_path;

    // NOTE(erick): We could have a race condition where the file is
    // create (or modified, renamed, deleted, etc) after this check
    // and before we actually open the file. It would be safer to
    // just open the file and then check if we succeed.
    if(!file->can_override && file_exists(output_file_path)) {
        fprintf(stderr, "File: %s already exists.\n", output_file_path);
        fprintf(stderr, "This file was ignored. To override it, please use '-o'\n");
        return;
    }

    if(file->replace == REPLACE_WITH_NAME) {
        file->replace_string = make_replace_str(file->filename);
    }

    if(file->replace == DONT_REPLACE) {
        copy_without_replacement(template_file_path, output_file_path);
    } else {
        FILE* template_file = fopen(template_file_path, "r");
        //TODO(erick): modify exit_on_error to use va
        if(!template_file) {
            fprintf(stderr, "Could not open the file: \"%s\"\n", template_file_path);
            exit(-1);
        }

       FILE* output_file = fopen(output_file_path, "w");
        //TODO(erick): modify exit_on_error to use va
        if(!output_file) {
            fprintf(stderr, "Could not open the file: \"%s\"\n", output_file_path);
            exit(-1);
        }

        StringBuffer string_buffer;
        string_buffer_init(&string_buffer);

        while(string_buffer_read_line(template_file, &string_buffer)) {
            char* stub_pos = strstr(string_buffer.buffer_data, STUB_STR);
            if(!stub_pos) {
                fprintf(output_file, "%s\n", string_buffer.buffer_data);
            } else {
                *stub_pos = '\0';
                fprintf(output_file, "%s", string_buffer.buffer_data);
                fprintf(output_file, "%s", file->replace_string);
                fprintf(output_file, "%s\n", stub_pos + strlen(STUB_STR));
            }
        }

        fclose(template_file);
        fclose(output_file);
    }

    if(file->replace == REPLACE_WITH_NAME) {
        free(file->replace_string);
    }

    // NOTE(erick): Copying mode bits from one file the other
    if(stat(template_file_path, &stat_buffer)) {
        // TODO(erick): Replace with the better version of exit_on_error
        fprintf(stderr, "Could not get the mode of %s\n", template_file_path);
        return;
    }

    mode_t file_mode = stat_buffer.st_mode;
#if DEBUG
    printf("Mode of %s is %o\n", template_file_path, file_mode);
#endif

    if(chmod(output_file_path, file_mode)) {
        fprintf(stderr, "Could not set the mode of %s\n", output_file_path);
        return;
    }
}

int main(int argc, char** argv) {
#define MAX_DIR_NAME (MAXNAMLEN + 1)
    char template_dir_name_buffer[MAX_DIR_NAME];
    DIR* template_dir = NULL;

    template_dir = get_template_dir(template_dir_name_buffer);

    if(!template_dir) {
        exit_on_error("Couldn't open the template directory\n");
    }

    //Begin processing the program arguments
    int files_to_output_count = 0;
    size_t file_names_length = 0;
    char* destination_dir = ".";

    // NOTE: First pass through the arguments.
    //       A two pass strategy has been chosen so we can allocate the
    //       right amount of memory

    for(int i = 1; i < argc; i++) {
        char* current_argument = argv[i];
        if(starts_with(current_argument, '-')) { // It's an option
            current_argument++;
            switch(*current_argument) {
            case 'd' :
                i++;
                eat_argument(argc, argv, i, &destination_dir);
                break;
            case 'o' :
                break;
            case 'r' :
                break;
            case 'e' : // Fallthrough and ignore both the -e and the extension
            case 'R' :
                i++; // We have the replace string to process
                break;
            default :
                exit_on_error("unknown option.");
            }
            // Option processed, can advance loop
            continue;
        }

        //Default behavior (Treat as a file)
        files_to_output_count++;
        file_names_length += strlen(current_argument) + 1;
    }

    // Can we access the destination directory?
    if(access(destination_dir, F_OK)) {
        exit_on_error("The destination directory does not exist or\n");
    }

    // Allocating the right amount of memory
    int destination_dir_len = strlen(destination_dir) + 1;
    file_data* files_to_output = (file_data*) malloc(files_to_output_count
                                                     * sizeof(file_data));

    // NOTE(erick): If the destination file is absolute we won't
    // concatenate the destination_dir path which means that we
    // probably we need less memory that we are actually allocating.
    // But whatever!
    size_t size_to_allocate =
        files_to_output_count * destination_dir_len + file_names_length;
    char* filenames_memory = (char*) malloc(size_to_allocate);

    if(!files_to_output || !filenames_memory) {
        exit_on_error("Failed to allocate memory");
    }

    // All the memory we need was allocated

    // NOTE: Second pass through the arguments happens here
    fill_files_to_output_paths(argc, argv,
                               files_to_output,
                               destination_dir,
                               filenames_memory);


    get_files_extensions(files_to_output, files_to_output_count);
    get_files_names(files_to_output, files_to_output_count);

    get_template_files(files_to_output, files_to_output_count,
                       template_dir, template_dir_name_buffer);
    closedir(template_dir);

    for(int file_index = 0;
        file_index < files_to_output_count;
        file_index++) {

        copy_file(files_to_output + file_index);
    }

    return 0;
}

bool is_absolute_path(char* path) {
    if(path == NULL) {
         return FALSE;
    }

    // NOTE(erick): We only support Unix-like paths i.e. paths starting with '/'
    return path[0] == '/';
}
