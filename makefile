CFLAGS = -D_FILE_OFFSET_BITS=64

all:
	gcc ds_manip.c -c -o ds_manip.o
	gcc ds_manip.o ssfs.c `pkg-config fuse --cflags --libs` $(CFLAGS) -o ssfs 
	@echo 'To Mount: ./ssfs -f [mount point]'

clean:
	rm *.o