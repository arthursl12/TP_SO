#define FUSE_USE_VERSION 30
#define BUFSZ 1024
#define CLIENT_MOD_FILENAME "lastmod_client.date"
#define CLIENT_HD_FILENAME "harddisk_client.dat"

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <stdbool.h>

#include "serverfs.h"
#include "ds_manip.h"

/* Client socket */
int s;
struct sockaddr_storage storage;
bool changed_but_not_in_server;
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
	if (!changed_but_not_in_server){
		// No local changes, just update from server
    	printf( "[getattr] Getting update, if needed\n" );
		update_if_needed(&s);
	}else{
		// We have local changes that must be sent to server
		// We'll assume no concurrent modifications are made
    	printf( "[getattr] Local changes must be sent to server\n" );
	}

	// Get attributes
	printf( "\tAttributes of %s requested\n", path );

	FILE *fptr;
	fptr = fopen(CLIENT_HD_FILENAME, "rb+");

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
	if (strcmp(path, "/") == 0 || is_dir(path) == 1){
	    // Root or any directory
		st->st_mode = S_IFDIR | 0755;   // It's a directory | permission bits
		st->st_nlink = 2; // Why "two" hardlinks instead of "one"? 
						  // The answer is here:
						  // http://unix.stackexchange.com/a/101536
	}else if (is_file(path) == 1){
		// Files
		st->st_mode = S_IFREG | 0644;
		st->st_nlink = 1;
		st->st_size = 1024;
	}else{
		// There is no such file or directory
		return -ENOENT;
	}

	// Save in harddisk file
    fseek(fptr, 0, SEEK_SET);
	fwrite(&dir_list, sizeof(dir_list), 1, fptr);
    fseek(fptr, sizeof(dir_list), SEEK_CUR);
	fwrite(&files_list, sizeof(files_list), 1, fptr);
	fseek(fptr, sizeof(files_list), SEEK_CUR);
	fwrite(&files_content, sizeof(files_content), 1, fptr);
	fseek(fptr, sizeof(files_content), SEEK_CUR);
	fwrite(&curr_dir_idx, sizeof(curr_dir_idx), 1, fptr);
	fseek(fptr, sizeof(curr_dir_idx) ,SEEK_CUR);
	fwrite(&curr_file_idx, sizeof(curr_file_idx), 1, fptr);
	fseek(fptr, sizeof(curr_file_idx) ,SEEK_CUR);
	fwrite(&curr_file_content_idx, sizeof(curr_file_content_idx), 1, fptr);
	fclose(fptr);

	// Update this file if needed
	if (!changed_but_not_in_server){
    	printf( "[getattr] We've just updated our harddisk\n" );
	}else{
		// We have local changes that must be sent to server
		// We'll assume no concurrent modifications are made
    	printf( "[getattr] Local changes must be sent to server\n" );
		// changed_but_not_in_server = false
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
	printf( "[readdir] Getting The List of Files of %s\n", path );
    
    // Current directory and parent directory as available entries
    // Known Unix convention
    filler(buffer, ".", NULL, 0 );      //Current
	filler(buffer, "..", NULL, 0 );     //Parent
    
        
    if ( strcmp( path, "/" ) == 0 ){
        // Root directory
		printf("\n%s contem os diretorios:\n", path);

		char *formatted_name;

		for(int i = 1; i < DIR_NUMBER + 1; i++){
			if(dir_list[0].sub_dirs[i] == 1){
				printf("sub_dirs[%i] = %i -> %s\n", i, dir_list[0].sub_dirs[i], dir_list[i].name);
				formatted_name = format_name(dir_list[i].name);
				printf("\tFomatted name: %s\n", formatted_name);
				// filler(buffer, dir_list[i].name, NULL, 0);
				filler(buffer, formatted_name, NULL, 0);
			}
		}
		
		printf("%s contem os arquivos:\n", path);

		for(int i = 0; i < FILE_NUMBER; i++){
			if(dir_list[0].files[i] == 1){
				printf("files[%i] = %i -> %s\n", i, dir_list[0].files[i], files_list[i]);
				formatted_name = format_name(files_list[i]);
				printf("\tFomatted name: %s\n", format_name(files_list[i]));
				// filler(buffer, files_list[i], NULL, 0);
				filler(buffer, formatted_name, NULL, 0);
			}
		}
		printf("\n");
	}else{
		path++;
		int dir_idx = get_directory_index(path);
		char *formatted_name;

		printf("\n%s contem os diretorios:\n", path);

		for(int i = 1; i < DIR_NUMBER + 1; i++){
			if(dir_list[dir_idx].sub_dirs[i] == 1){
				printf("sub_dirs[%i] = %i -> %s\n", i, dir_list[dir_idx].sub_dirs[i] ,dir_list[i].name);
				formatted_name = format_name(dir_list[i].name);
				printf("\tFomatted name: %s\n", formatted_name);
				// filler(buffer, "subdir", NULL, 0);
				filler(buffer, formatted_name, NULL, 0);
			}
		}
		
		printf("%s contem os arquivos:\n", path);

		for(int i = 0; i < FILE_NUMBER; i++){
			if(dir_list[dir_idx].files[i] == 1){
				printf("files[%i] = %i -> %s\n", i, dir_list[dir_idx].files[i], files_list[i]);
				formatted_name = format_name(files_list[i]);
				printf("\tFomatted name: %s\n", formatted_name);
				// filler(buffer, "file", NULL, 0);
				filler(buffer, formatted_name, NULL, 0);
			}
		}
		printf("\n");
	}

	return 0;
}

static int do_open(const char *path, struct fuse_file_info *fi) {
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

	printf( "[read] Called to read %s\n", path);
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
	printf( "[mkdir] Called\n" );
	path++;
	add_dir( path );
	printf( "[mkdir] Folder %s created sucessfully \n", path);
	changed_but_not_in_server = true;
	return 0;
}

/*
Creates a new file with given path
Returns 0 on sucess.
*/
static int do_mknod( const char *path, mode_t mode, dev_t rdev ){
	// mode specifies the permission bits and type of the new file
	// rdev should be specified if the new file is a device file
	printf( "[mknod] Called\n" );
	path++;
	add_file( path );
	printf( "[mknod] File %s created sucessfully \n", path);
	changed_but_not_in_server = true;
	return 0;
}

/*
Writes given content in buffer to the start of file in given path.
Returns the number of written bytes
*/
static int do_write(const char *path, const char *buffer, 
					size_t size, off_t offset, struct fuse_file_info *info){
	printf( "[write] Called\n");
	write_to_file( path, buffer );
	printf( "[write] Written to file %s sucessfully \n", path);
	changed_but_not_in_server = true;
	return size;
}


static struct fuse_operations operations = {
    .getattr	= do_getattr,
    .readdir	= do_readdir,
	.read		= do_read,
    .mkdir		= do_mkdir,
    .mknod		= do_mknod,
    .write		= do_write,
	.open		= do_open,
};

int main( int argc, char *argv[] ){
	// Harddisk creation if it doesn't exist    
    if(access(CLIENT_HD_FILENAME, F_OK ) == 0){
        // file exists
    }else{
        // file doesn't exist, we'll create a blank one
        FILE *fptr;
	    fptr = fopen(CLIENT_HD_FILENAME, "w");
        fclose(fptr);
    }

	// Harddisk initialization in memory
	FILE *fptr;
	fptr = fopen(CLIENT_HD_FILENAME, "rb+");

	if(fptr == NULL){
		printf("\n Error opening harddisk file\n");
		exit(1);
	} else{
		fseek(fptr, 0, SEEK_END);
		int size = ftell(fptr);

		if(size == 0){
			INIT_DIR(root);
			strcpy(root.name, "/");
			dir_list[0] = root;

			printf("\n\n - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n" );
			printf("\tFILE SYSTEM MOUNTED\n\n");
		}else{
			fseek(fptr, 0, SEEK_SET);
			while(fread(&dir_list, sizeof(dir_list), 1, fptr) == sizeof(dir_list));
			fseek(fptr, sizeof(dir_list), SEEK_CUR);
			while(fread(&files_list, sizeof(files_list), 1, fptr) == sizeof(files_list));
			fseek(fptr, sizeof(files_list), SEEK_CUR);
			while(fread(&files_content, sizeof(files_content), 1, fptr) == sizeof(files_content));
			fseek(fptr, sizeof(files_content), SEEK_CUR);
			while(fread(&curr_dir_idx, sizeof(curr_dir_idx), 1, fptr) == sizeof(curr_dir_idx));
			fseek(fptr, sizeof(curr_dir_idx) , SEEK_CUR);
			while(fread(&curr_file_idx, sizeof(curr_file_idx), 1, fptr) == sizeof(curr_file_idx));
			fseek(fptr, sizeof(curr_file_idx) , SEEK_CUR);
			while(fread(&curr_file_content_idx, sizeof(curr_file_content_idx), 1, fptr) ==\
			      sizeof(curr_file_content_idx));
			printf("\n\n - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -\n" );
			printf("\tFILE SYSTEM RE-MOUNTED\n\n");
		}
	}

	printf("Lista de diretorios:\n");

	for(int curr_idx = 1; curr_idx <= curr_dir_idx; curr_idx++)
		printf("\t%s\n", dir_list[curr_idx].name);

	printf("Lista de arquivos:\n");

	for(int curr_idx = 0; curr_idx <= curr_file_idx; curr_idx++)
		printf("\t%s\n", files_list[curr_idx]);

	printf("\n"); 	

	fclose(fptr);


	// Client socket initialization
	// struct sockaddr_storage storage;
	char* ip = "127.0.0.1";
	char* port = "51511";
    if (addrparse(ip, port, &storage) != 0){
        logexit("ipport");
    }

    // Conexão com o servidor
    // int s = socket(storage.ss_family, SOCK_STREAM, 0);
    s = socket(storage.ss_family, SOCK_STREAM, 0);

    if (s == -1) { logexit("socket");}
    struct sockaddr* addr = (struct sockaddr*)(&storage);
    if (connect(s, addr, sizeof(storage)) != 0){ 
		logexit("connect"); 
		exit(EXIT_FAILURE);
	}
    char addrstr[BUFSZ];       
    addrtostr(addr, addrstr, BUFSZ);        // Imprimir o IP do servidor
    printf("Sucessfully connected to %s.\n", addrstr);


	return fuse_main( argc, argv, &operations, NULL );
}

