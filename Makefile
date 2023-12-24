CFLAGS = -Wall

copyit: copyit.c
	gcc $(CFLAGS) -o copyit copyit.c
	chmod +x copyit
