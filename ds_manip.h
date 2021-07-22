#ifndef DS_MANIP_H
#define DS_MANIP_H
#pragma once

/*
Data Structures to store filesystem information.
For now only simple arrays
*/
char dir_list[ 256 ][ 256 ]; 	// Name of directories created
// int curr_dir_idx = -1;

char files_list[ 256 ][ 256 ];	// Name of files created
// int curr_file_idx = -1;

char files_content[ 256 ][ 256 ];	// Content of files created
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

#endif /* DS_MANIP */

