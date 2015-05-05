CC = g++
CFLAGS =
OBJS = main.o
PROG = mydu
 
.SUFFIXES: .c .o
 
$(PROG): $(OBJS)
	$(CC) -o $@ $(OBJS)
 
.c.o:
	$(CC) -c -o $@ $<
 
clean:
	rm *.o $(PROG)
