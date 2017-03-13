# C version (default) -> "make" or "make yacc=0"
# yacc version -> "make yacc=1"

CC			= cc
CFLAGS	= -g -ansi -pedantic -Wall
LDFLAGS	=

OBJS_DEFAULT = codegen.o \
			compile.o \
			getSource.o \
			main.o \
			table.o

OBJS_YACC = codegen.o \
			table.o \
			lex.yy.o \
			y.tab.o

OBJS = $(OBJS_DEFAULT)

ifdef yacc
ifneq ($(yacc), 0)
  OBJS = $(OBJS_YACC)
endif
endif

.SUFFIXES	: .o .c

all : pl0d

clean	:
	rm -f pl0d pl0d_yacc *~ *.o lex.yy.c y.tab.*

.c.o :
	$(CC) $(CFLAGS) -c $<

pl0d : $(OBJS)
	$(CC) -o $@ $(OBJS)

getSource.h : table.h
codegen.o : 	codegen.h getSource.h table.h
compile.o : 	codegen.h getSource.h table.h
getSource.o : getSource.h
main.o :			getSource.h
table.o : 		getSource.h
lex.yy.o :		lex.yy.c
y.tab.o :			y.tab.c

y.tab.c : pl0.y
	bison -d -y $<

lex.yy.c : pl0.l y.tab.c
	flex -l $<
