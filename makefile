CFLAGS = -D_FILE_OFFSET_BITS=64

all:
	gcc -Wall -c packets.c
	gcc -Wall client.c packets.o -lpthread -o client
	gcc -Wall server-mt.c packets.o -lpthread -o server-mt

	gcc -Wall ds_manip.c -c -o ds_manip.o
	gcc -Wall ds_manip.o ssfs.c `pkg-config fuse --cflags --libs` $(CFLAGS) -o ssfs 
	@echo 'To Mount: ./ssfs -f [mount point]'

clean:
	rm *.o
	rm client server-mt ssfs