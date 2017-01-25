
CC		= cc
CFLAGS	= -g -ansi -pedantic -Wall
LDFLAGS	=

OBJS	=	codegen.o \
			compile.o \
			getSource.o \
			main.o \
			table.o

.SUFFIXES	: .o .c

.c.o	: 
	$(CC) $(CFLAGS) -c $<

pl0d	: $(OBJS)
	$(CC) -o $@ $(OBJS)

clean	:
	rm -f pl0d *~ *.o
