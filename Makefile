CC=gcc
CFLAGS=-g

SRCDIR=src
INCLDIR=include

pingpong: $(SRCDIR)/pingpong.c 
	$(CC) $(CFLAGS) -I$(INCLDIR) $(SRCDIR)/pingpong.c -o pingpong 

clean:
	rm pingpong 
