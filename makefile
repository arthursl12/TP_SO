all:
	gcc ssfs.c -o ssfs `pkg-config fuse --cflags --libs`
	@echo 'To Mount: ./ssfs -f [mount point]'
