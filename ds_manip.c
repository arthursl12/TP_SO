#include <string.h>

#include "ds_manip.h"

int curr_dir_idx = -1;
int curr_file_idx = -1;
int curr_file_content_idx = -1;

/*
Adds a directory name to the list
Returns void
*/
void add_dir(const char *dir_name){
	curr_dir_idx++;
	strcpy( dir_list[ curr_dir_idx ], dir_name );
}

/*
Returns 1 if given path is a directory.
Returns 0 otherwise.

Here we see a limitation of current data structure. We only store names in 
the list not complete paths.
*/
int is_dir(const char *path){
	path++; // Eliminating "/" in the path
	
	for (int curr_idx = 0; curr_idx <= curr_dir_idx; curr_idx++ )
		if ( strcmp( path, dir_list[ curr_idx ] ) == 0 )
			return 1;
	return 0;
}

/*
Similar to add_dir. Adds a file to the filesystem
Returns void
*/
void add_file(const char *filename){
	curr_file_idx++;
	strcpy( files_list[ curr_file_idx ], filename );
	
	curr_file_content_idx++;
	strcpy( files_content[ curr_file_content_idx ], "" );
}

/*
Returns 1 if given path is a file.
Returns 0 otherwise.

Here we have the same limitation reported in 'is_dir'
*/
int is_file( const char *path ){
	path++; // Eliminating "/" in the path
	
	for ( int curr_idx = 0; curr_idx <= curr_file_idx; curr_idx++ )
		if ( strcmp( path, files_list[ curr_idx ] ) == 0 )
			return 1;
	
	return 0;
}
/*
Returns file index, if it exists.
Returns -1 if no such file can be found.
*/
int get_file_index(const char *path){
	path++; // Eliminating "/" in the path
	
	for ( int curr_idx = 0; curr_idx <= curr_file_idx; curr_idx++ )
		if ( strcmp( path, files_list[ curr_idx ] ) == 0 )
			return curr_idx;
	
	return -1;
}

/*
Write some content to the file in the given path
If this file doesn't exists, nothing is done
*/
void write_to_file( const char *path, const char *new_content ){
	int file_idx = get_file_index( path );
	
	if ( file_idx == -1 ) // No such file
		return;
		
	strcpy( files_content[ file_idx ], new_content ); 
}
