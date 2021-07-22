#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "ds_manip.h"

/* Based on tutorials:
https://www.maastaar.net/fuse/linux/filesystem/c/2016/05/21/writing-a-simple-filesystem-using-fuse/
https://www.maastaar.net/fuse/linux/filesystem/c/2019/09/28/writing-less-simple-yet-stupid-filesystem-using-FUSE-in-C/

Notes: 
1) The filesystem is not persistant yet. So as of now it is an 
in-memory only system, meaning that everything stored within it (including
directories and files created) will be lost once it is unmounted.
2) The system is only capable of handling root-level objects (for now). In order
to use subdirectories and such, more complex data structures should be created.
*/

/*
Data Structures to store filesystem information.
For now only simple arrays
*/
// extern char dir_list[ 256 ][ 256 ]; 	// Name of directories created
// extern int curr_dir_idx = -1;

// extern char files_list[ 256 ][ 256 ];	// Name of files created
// extern int curr_file_idx = -1;

// extern char files_content[ 256 ][ 256 ];	// Content of files created
// extern int curr_file_content_idx = -1;


/*
GNU's definitions of the attributes 
(http://www.gnu.org/software/libc/manual/html_node/Attribute-Meanings.html):
    st_uid: 	The user ID of the file’s owner.
	st_gid: 	The group ID of the file.
	st_atime: 	This is the last access time for the file.
	st_mtime: 	This is the time of the last modification to the contents 
				of the file.
	st_mode: 	Specifies the mode of the file. This includes file type 
				information (see Testing File Type) and the file permission
				bits (see Permission Bits).
	st_nlink: 	The number of hard links to the file. This count keeps track of 
				how many directories have entries for this file. If the count is
				ever decremented to zero, then the file itself is discarded as 
				soon as no process still holds it open. Symbolic links are not 
				counted in the total.
	st_size:	This specifies the size of a regular file in bytes. For files 
				that are really devices this field isn’t usually meaningful. For
				symbolic links this specifies the length of the file name the 
				link refers to.
*/

/*
Called when the system asks the FS for the attributes of a specific file or 
directory

Receives filepath and stat datatype
Returns 0 on success, -ENOENT when it doesn't exist
*/
static int do_getattr(const char *path, struct stat *st){
    printf( "[getattr] Called\n" );
	printf( "\tAttributes of %s requested\n", path );

    // Define some stat attributes
	st->st_uid = getuid();      // Owner of the file (current user)
	st->st_gid = getgid();      // Owner group of the files/directories 
								// (current user group)
	st->st_atime = time( NULL );    // Last acess time
	st->st_mtime = time( NULL );    // Last modification time
	//Both fields must be Unix time
	
	// st_mode: it is a file, directory or another thing.
	//      	Also defines Unix permission bits
	// st_nlink: qtd of hardlinks to file/directory
	// st_size: filesize in bytes
	if (strcmp(path, "/") == 0 || is_dir( path ) == 1){
	    // Root or any directory
		st->st_mode = S_IFDIR | 0755;   // It's a directory | permission bits
		st->st_nlink = 2; // Why "two" hardlinks instead of "one"? 
						  // The answer is here:
						  // http://unix.stackexchange.com/a/101536
	}else if (is_file( path ) == 1){
		// Files
		st->st_mode = S_IFREG | 0644;
		st->st_nlink = 1;
		st->st_size = 1024;
	}else{
		// There is no such file or directory
		return -ENOENT;
	}
	return 0;
}

/*
Called when the system asks the FS to list the files and subdirectories ('ls')

Receives directory path, buffer to fill in the requested files under it, 
filler to fill the buffer provided by FUSE
Returns 0 on success, -1 otherwise
*/

static int do_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, 
 					  off_t offset, struct fuse_file_info *fi ){
    printf( "--> Getting The List of Files of %s\n", path );
    
    // Current directory and parent directory as available entries
    // Known Unix convention
    filler(buffer, ".", NULL, 0 );      //Current
	filler(buffer, "..", NULL, 0 );     //Parent
    
        
    if ( strcmp( path, "/" ) == 0 ){
        // Root directory

		// Adds all directories to the buffer
		for (int curr_idx = 0; curr_idx <= curr_dir_idx; curr_idx++)
			filler( buffer, dir_list[ curr_idx ], NULL, 0 );
		
		// Adds all files to the buffer
		for (int curr_idx = 0; curr_idx <= curr_file_idx; curr_idx++)
			filler( buffer, files_list[ curr_idx ], NULL, 0 );
	}
	return 0;
}

/*
Allows the system to read the content of a specific file

Receives file path, buffer is the chunk which the system is interested in, 
size of said chunk, offset is where we'll start reading

Returns how many bytes have been successfully read.
Returns -1 if file is not found
*/
static int do_read(const char *path, char *buffer, size_t size, 
				   off_t offset, struct fuse_file_info *fi ){

	int file_idx = get_file_index( path );
	if (file_idx == -1)
		return -1;

	// Copy content to buffer and send it to the system
	char *content = files_content[ file_idx ];
	memcpy(buffer, content + offset, size);
	return strlen(content) - offset;
}

/*
Creates a new directory with given path
Returns 0 on sucess.
*/
static int do_mkdir(const char *path, mode_t mode){
	path++;
	add_dir( path );
	
	return 0;
}

/*
Creates a new file with given path
Returns 0 on sucess.
*/
static int do_mknod( const char *path, mode_t mode, dev_t rdev ){
	// mode specifies the permission bits and type of the new file
	// rdev should be specified if the new file is a device file
	path++;
	add_file( path );
	
	return 0;
}

/*
Writes given content in buffer to the start of file in given path.
Returns the number of written bytes
*/
static int do_write(const char *path, const char *buffer, 
					size_t size, off_t offset, struct fuse_file_info *info){
	write_to_file( path, buffer );
	
	return size;
}


static struct fuse_operations operations = {
    .getattr	= do_getattr,
    .readdir	= do_readdir,
	.read		= do_read,
    .mkdir		= do_mkdir,
    .mknod		= do_mknod,
    .write		= do_write,
};

int main( int argc, char *argv[] ){
	return fuse_main( argc, argv, &operations, NULL );
}

