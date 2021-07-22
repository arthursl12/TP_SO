#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

/* Based on tutorial:
https://www.maastaar.net/fuse/linux/filesystem/c/2016/05/21/writing-a-simple-filesystem-using-fuse/
*/


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
Called when the system asks the FS for the attributes of a specific file

Receives filepath and stat datatype
Returns 0 on success, -1 otherwise
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
	if (strcmp(path, "/") == 0 ){
	    // Root directory
		st->st_mode = S_IFDIR | 0755;   // It's a directory | permission bits
		st->st_nlink = 2; // Why "two" hardlinks instead of "one"? 
						  // The answer is here:
						  // http://unix.stackexchange.com/a/101536
	}else{
	    // All files other than root directory
		st->st_mode = S_IFREG | 0644;   // It's a regular file | permission bits
		st->st_nlink = 1;
		st->st_size = 1024;
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
        // Two example files in the initial example
		filler( buffer, "file54", NULL, 0 );
		filler( buffer, "file349", NULL, 0 );
	}
}

/*
Allows the system to read the content of a specific file

Receives file path, buffer is the chunk which the system is interested in, 
size of said chunk, offset is where we'll start reading
Returns how many bytes have been successfully read
*/
static int do_read(const char *path, char *buffer, size_t size, 
				   off_t offset, struct fuse_file_info *fi ){
    char file54Text[] = "Hello World From File54!";
	char file349Text[] = "Hello World From File349!";
	char *selectedText = NULL; // Content to be returned to the system

    if ( strcmp(path, "/file54" ) == 0 ){
		selectedText = file54Text;
	}else if ( strcmp(path, "/file349" ) == 0 ){
		selectedText = file349Text;
	}else{
	    // Error: could not find the requested file
		return -1;
    }
    
    // Copy content to buffer and send it to the system
    memcpy( buffer, selectedText + offset, size );
	return strlen( selectedText ) - offset;
}

static struct fuse_operations operations = {
    .getattr	= do_getattr,
    .readdir	= do_readdir,
    .read	= do_read,
};

int main( int argc, char *argv[] ){
	return fuse_main( argc, argv, &operations, NULL );
}

