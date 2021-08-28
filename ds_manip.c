#include <string.h>

#include "ds_manip.h"

int curr_dir_idx = 0; // Store user directories beggining from 1
int curr_file_idx = -1;
int curr_file_content_idx = -1;

/*
Adds a directory name to the list
Returns void
*/
void add_dir(const char *dir_name){
	INIT_DIR(d);
	char *parent_name = get_parent_name(dir_name);
	int parent_index = get_directory_index(parent_name);
	strcpy(d.name, dir_name);
	curr_dir_idx++;
	dir_list[curr_dir_idx] = d;
	dir_list[parent_index].sub_dirs[curr_dir_idx] = 1;
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
		if ( strcmp( path, dir_list[curr_idx].name ) == 0 )
			return 1;
	return 0;
}

/*
Similar to add_dir. Adds a file to the filesystem
Returns void
*/
void add_file(const char *filename){
	char *parent_name = get_parent_name(filename);
	int parent_index = get_directory_index(parent_name);
	curr_file_idx++;
	strcpy( files_list[ curr_file_idx ], filename );
	dir_list[parent_index].files[curr_file_idx] = 1;
	
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

/*
Find parent directory of file/subdirectory
*/
char *get_parent_name(const char *name){
	int size = sizeof(name);
	int index = 0;

	//Locate last "/" in name
	for(int i = size - 1; i >= 0; i--){
		if(name[i] == '/'){
			index = i;
			break;
		}
	}

	// If theres no "/" in name, parent is root
	if(index == 0){
		return "/";
	}

    // If there is a "/" in name, parent is string before last "/"
	char aux[index];
	
	for(int i = 0; i < index; i++)
		aux[i] = name[i];

	char *parent_name = aux;

	return parent_name;
	
}

/*
Finds position of a directory in dir_list
*/
int get_directory_index(const char *name){
	for(int i = 0; i < DIR_NUMBER + 1; i++){
		if(strcmp(dir_list[i].name, name) == 0)
			return i;
	}
}

/*
Removes parent directory name from file/subdirectory name
*/
char *format_name(const char *name){
	int size = sizeof(name);
	int index = 0;

	// Locate last "/" in name
	for(int i = size - 1; i >= 0; i--){
		if(name[i] == '/'){
			index = i;
			break;
		}
	}

	char *formatted_name;

	if(index == 0) {
		formatted_name = name;
		return formatted_name;
	}else{
		// Return string after last "/"
		formatted_name = name + (index + 1);
		return formatted_name;
	}
}