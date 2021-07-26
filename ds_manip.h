#ifndef DS_MANIP_H
#define DS_MANIP_H
#pragma once

#define DIR_NUMBER 256
#define FILE_NUMBER 256
#define STR_LENGTH 256

#define INIT_DIR(D) DIR D = {.name = "", .sub_dirs = {0}, .files = {0}}

typedef struct Directory{
    char name[STR_LENGTH]; // Directory name
    int sub_dirs[DIR_NUMBER + 1]; // If sub_dirs[i] = 1, directory in dir_list[i] belongs to this
                                  // directory. If not, sub_dirs[i] = 0
    int files[FILE_NUMBER];// If files[i] = 1, file in files_list[i] belongs to this directory.
                           // If not, files[i] = 0
} DIR;

/*
Data Structures to store filesystem information.
For now only simple arrays
*/

// dir_list[0] stores root structure of root directory
// Directories created by the user are stored beggining from dir_list[1]
DIR dir_list[DIR_NUMBER + 1];
//char dir_list[DIR_NUMBER][STR_LENGTH]; 	// Name of directories created
// int curr_dir_idx = -1;

char files_list[FILE_NUMBER][STR_LENGTH];	// Name of files created
// int curr_file_idx = -1;

char files_content[FILE_NUMBER][STR_LENGTH];	// Content of files created
// int curr_file_content_idx = -1;

extern int curr_dir_idx;
extern int curr_file_idx;
extern int curr_file_content_idx;


void add_dir(const char *dir_name);
int is_dir(const char *path);
void add_file(const char *filename);
int is_file( const char *path );
int get_file_index(const char *path);
void write_to_file(const char *path, const char *new_content);
char *get_parent_name(const char *name);
int get_directory_index(const char *name);
char *format_name(const char *name);

#endif /* DS_MANIP */

