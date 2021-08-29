CFLAGS = -D_FILE_OFFSET_BITS=64 

all:
	gcc -Wall -c packets.c
	gcc -Wall -c filedate.c
	gcc -Wall -c serverfs.c

	gcc -Wall server-mt.c packets.o filedate.o -lpthread -o server-mt

	
	gcc ds_manip.c -c -o ds_manip.o
	gcc -Wno-unused-variable -Wall ds_manip.o filedate.o packets.o ssfs.c `pkg-config fuse --cflags --libs` $(CFLAGS) -o ssfs  
	@echo 'To Mount: ./ssfs -f [mount point]'

clean:
	rm *.o