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
GNU's definitions of the attributes (http://www.gnu.org/software/    libc/manual/html_node/Attribute-Meanings.html):
    st_uid: 	The user ID of the file’s owner.
	st_gid: 	The group ID of the file.
	st_atime: 	This is the last access time for the file.
	st_mtime: 	This is the time of the last modification to the contents of the file.
	st_mode: 	Specifies the mode of the file. This includes file type information (see Testing File Type) and the file permission bits (see Permission Bits).
	st_nlink: 	The number of hard links to the file. This count keeps track of how many directories have entries for this file. If the count is ever decremented to zero, then the file itself is discarded as soon as no process still holds it open. Symbolic links are not counted in the total.
	st_size:	This specifies the size of a regular file in bytes. For files that are really devices this field isn’t usually meaningful. For symbolic links this specifies the length of the file name the link refers to.
*/

/*
Called when the system asks the FS for the attributes of a specific file

Receives filepath and stat datatype
Returns 0 on sucess, -1 on failure
*/
static int do_getattr(const char *path, struct stat *st){
    printf( "[getattr] Called\n" );
	printf( "\tAttributes of %s requested\n", path );

    // Define some stat attributes
	st->st_uid = getuid();      // Owner of the file (current user)
	st->st_gid = getgid();      // Owner group of the files/directories (current user group)
	st->st_atime = time( NULL );    // Last acess time
	st->st_mtime = time( NULL );    // Last modification time
	//Both fields must be Unix time
	
	// st_mode: it is a file, directory or another thing.
	//      Also defines Unix permission bits
	// st_nlink: # of hardlinks to file/directory
	// st_size: filesize in bytes
	if (strcmp(path, "/") == 0 ){
	    // Root directory
		st->st_mode = S_IFDIR | 0755;   // It's a directory | permission bits
		st->st_nlink = 2; // Why "two" hardlinks instead of "one"? The answer is here: http://unix.stackexchange.com/a/101536
	}else{
	    // All files other than root directory
		st->st_mode = S_IFREG | 0644;   // It's a regular file | permission bits
		st->st_nlink = 1;
		st->st_size = 1024;
	}
	return 0;
}


