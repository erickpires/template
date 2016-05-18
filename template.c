/* This program is a C implementation of the template script
 * written by Ian Brunelli.
 * It has been developed for learning purposes and to add some
 * features that the original and lazy developer couldn't make.
 *
 * You may do what you want with this code, I don't mind. Also,
 * no warranties are involved.
 *
 * The program receives a list of filenames as its standard parameters
 * for each of the filenames it search the template directory for a file
 * with the same extension and then makes a copy of this file in the
 * destination directory.
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

#define starts_with(str,ch) (*str == ch)

void exit_on_error(char* msg){
	fprintf(stderr, msg);
	exit(-1);
}

// TODO: Error messages are terrible. Use something as var_args to improve it.
void eat_argument(int argc, char** argv, int index, char** dest) {
	if(index >= argc)
		exit_on_error("Too few arguments.\n");

	*dest = argv[index];
	argv[index] = NULL;
}

inline void trim_right(char* str){
	int len = strlen(str);
	{
		int i  = len - 1;

		while( i >= 0 &&
			 ( str[i] == '\n' || str[i] == '\t' || str[i] == ' '))
		{
			str[i--] = '\0';
		}
	}
}

inline int matches_file_format(char* filename, char* file_extension){
	int end_filename = strlen(filename);
	int end_file_extension = strlen(file_extension);

	int index_filename = end_filename - 1;
	int index_file_extension = end_file_extension - 1;

	while(filename[index_filename] == file_extension[index_file_extension]){
		if(index_file_extension == 0)
			return 1;
		if(index_filename == 0)
			return 0;

		index_filename--;
		index_file_extension--;
	}

	return 0;
}

inline char* copy_string(char* dest, char* src){
	//NOTE: copies all the characters from src to dest and
	//returns a pointer to last character of dest.
	while(*src)
		*dest++ = *src++;

	*dest++ = '\0';
	return dest;
}

void fill_files_to_output_paths(int argc, char** argv,
								file_data* files_to_output,
								char* destination_dir,
								char* filenames_memory){

	for(int arg_index = 1; arg_index < argc; arg_index++){
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
				files_to_output->replace = replace_with_name;
				break;
			case 'e' :
				arg_index++;
				eat_argument(argc, argv, arg_index, &(files_to_output->file_extension));
				break;
			case 'R' :
				files_to_output->replace = replace_with_argument;
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

		files_to_output->file_path = filenames_memory;
		filenames_memory =  copy_string(filenames_memory, destination_dir);
		filenames_memory =  copy_string(filenames_memory - 1, "/");
		filenames_memory =  copy_string(filenames_memory -1, current_argument);
		files_to_output++;
	}
}

void get_files_extensions(file_data* files_to_output, int files_to_output_count){

	for(int i = 0; i < files_to_output_count; i++){
		if(files_to_output[i].file_extension == NULL) {
			int pos = strlen(files_to_output[i].file_path) - 1;
			while(files_to_output[i].file_path[pos] != '.' && pos > 0) //TODO: extremely confusing
				pos--;

			if(pos == 0)
				exit_on_error("You must specify an extension.\n");

			files_to_output[i].file_extension = files_to_output[i].file_path + pos;
		}
	}
}

void get_files_names(file_data* files, int files_count,
					 int destination_dir_len){

	for(int i = 0; i < files_count; i++)
		files[i].filename = files[i].file_path + destination_dir_len;
}

DIR* get_template_dir(char* template_dir_name_buffer){

	FILE* xdg_result = NULL;
	//NOTE: Since we only have access to the stdin stream
	//      The error stream is redirected so we can read it.
	xdg_result = popen("xdg-user-dir TEMPLATES 2>&1", "r");

	if(!xdg_result)
		exit_on_error("Failed to use popen\n");

	fgets(template_dir_name_buffer, 256, xdg_result);

	pclose(xdg_result);

	if(strstr(template_dir_name_buffer, "sh:"))
		exit_on_error("Could not execute xdg-user-dir\n");

	trim_right(template_dir_name_buffer); // Remove the last new line

	return opendir(template_dir_name_buffer);
}

void get_template_files(file_data* files, int files_count,
						DIR* template_dir, char* template_dir_name){
	int i;
	int completed_files = 0;
	dir_ent* template_file_dir_ent;

	while((template_file_dir_ent = readdir(template_dir))){
		char* template_file_full_path = NULL;
		for(i = 0; i < files_count; i++){

			if(matches_file_format(template_file_dir_ent->d_name, files[i].file_extension)){

				if(template_file_full_path == NULL){

					template_file_full_path = (char*) malloc(
				    	strlen(template_dir_name) +
				    	strlen(template_file_dir_ent->d_name) +
				    	strlen("/") + 1);

				    if(!template_file_full_path)
				    	exit_on_error("Could not allocate memory");

				    strcpy(template_file_full_path, template_dir_name);
				    strcat(template_file_full_path, "/");
				    strcat(template_file_full_path, template_file_dir_ent->d_name);
				}

				files[i].template_file_path = template_file_full_path;
				completed_files++;

				if(completed_files == files_count)
					return;
			}
		}
	}

	if(files_count != completed_files){
		fprintf(stderr, "Couldn't find matches for the file(s):\n");
		{
			for(int i = 0; i < files_count; i++){
				if(files[i].template_file_path == NULL)
					fprintf(stderr, "  \"%s\"\n", files[i].filename);
			}
		}
		exit(-1);
	}
}

char* make_replace_str(char* filename){
	char* result = (char*) calloc(strlen(filename) + 1, sizeof(char));
	if(!result)
		exit_on_error("Could not allocate memory");

	for(int i = 0; i < strlen(filename); i++){
		if(isalpha(filename[i]))
			result[i] = filename[i] & (~0x20);
		else
			result[i] = '_';
	}
	return result;
}

inline bool file_exists(char* file_path) {
	return access(file_path, F_OK) == 0;
}

void copy_file(file_data* file){
	stat_buf stat_buffer;
	char* template_file_path = file->template_file_path;
	char* output_file_path = file->file_path;

	if(!file->can_override && file_exists(output_file_path)) {
		fprintf(stderr, "File: %s already exists.\n", output_file_path);
		fprintf(stderr, "This file was ignored. To override it, please use '-o'\n");
		return;
	}

	if(file->replace == replace_with_name)
		file->replace_string = make_replace_str(file->filename);

	FILE* template_file = NULL;
	FILE* output_file = NULL;
    template_file = fopen(template_file_path, "r");
    output_file = fopen(output_file_path, "w");

    //TODO: modify exit_on_error to use va
    if(!template_file){
    	fprintf(stderr, "Could not open the file: \"%s\"\n", template_file_path);
    	exit(-1);
    }
    if(!output_file){
    	fprintf(stderr, "Could not open the file: \"%s\"\n", output_file_path);
    	exit(-1);
    }

    char buffer[1024];

    while(!feof(template_file)){
    	if(!fgets(buffer, sizeof(buffer), template_file))
    		break;

    	if(file->replace != dont_replace){
    		//TODO: the case where the line is longer than 1024 character
    		//should be handle properly
    		char* stub_pos = strstr(buffer, STUB_STR);
    		if(!stub_pos)
    			fprintf(output_file, "%s", buffer);
    		else{
    			*stub_pos = '\0';
    			fprintf(output_file, "%s", buffer);
    			fprintf(output_file, "%s", file->replace_string);
    			fprintf(output_file, "%s", stub_pos + strlen(STUB_STR));
    		}

    	}
    	else
			fprintf(output_file, "%s", buffer);
    }

    fclose(template_file);
    fclose(output_file);

	if(file->replace == replace_with_name)
		free(file->replace_string);

    // Copying mode bits from one file the other
    if(stat(template_file_path, &stat_buffer)) {
    	// TODO: Replace with the better version of exit_on_error
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

int main(int argc, char** argv){
#define MAX_DIR_NAME (MAXNAMLEN + 1)
	char template_dir_name_buffer[MAX_DIR_NAME];
	DIR* template_dir = NULL;

	template_dir = get_template_dir(template_dir_name_buffer);

	if(!template_dir)
		exit_on_error("Couldn't open the template directory\n");

	//Begin processing the program arguments
	int files_to_output_count = 0;
	size_t file_names_length = 0;
	char* destination_dir = ".";

	// NOTE: First pass through the arguments.
	//       A two pass strategy has been chosen so we can allocate the
	//       right amount of memory

	for(int i = 1; i < argc; i++){
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
	if(access(destination_dir, F_OK))
		exit_on_error("The destination directory does not exist or\n");

	// Allocating the right amount of memory
	int destination_dir_len = strlen(destination_dir) + 1;
	file_data* files_to_output = (file_data*) malloc(files_to_output_count * sizeof(file_data));
	char* filenames_memory = (char*) malloc(files_to_output_count * destination_dir_len +
											file_names_length);

	if(! files_to_output || !filenames_memory)
		exit_on_error("Failed to allocate memory");
	// All the memory we need was allocated

	// NOTE: Second pass through the arguments happens here
	fill_files_to_output_paths(argc, argv, files_to_output, destination_dir, filenames_memory);


	get_files_extensions(files_to_output, files_to_output_count);
	get_files_names(files_to_output, files_to_output_count, destination_dir_len);

	get_template_files(files_to_output, files_to_output_count,
					   template_dir, template_dir_name_buffer);
	closedir(template_dir);

	for(int file_index = 0; file_index < files_to_output_count; file_index++) {
		copy_file(files_to_output + file_index);
	}

    return 0;
}
