#
# make option:
#   C version (default) -> "make" or "make yacc=0"
#   yacc version -> "make yacc=1"
#
# exe files:
#   pl0c: compile pl0 language into op file
#   pl0e: run op file

CC			= cc
CFLAGS	= -g -ansi -pedantic -Wall
LDFLAGS	=

ifdef yacc
  yacc = 1
else
	yacc = 0
endif

OBJS_COMMON = codegen.o table.o
OBJS_PL0C = $(OBJS_COMMON) pl0c_main.o

ifeq ($(yacc), 0)
  OBJS_PL0C += compile.o getSource.o
else
  OBJS_PL0C += lex.yy.o
  CFLAGS += -DYACC
endif

OBJS_PL0E = $(OBJS_COMMON) \
			getSource.o \
			pl0e_main.o

.SUFFIXES	: .o .c

all : pl0c pl0e

clean	:
	rm -f pl0c *~ *.o lex.yy.c y.tab.*

.c.o :
	$(CC) $(CFLAGS) -c $<

pl0c : $(OBJS_PL0C)
	$(CC) -o $@ $(OBJS_PL0C)

pl0e : $(OBJS_PL0E)
	$(CC) -o $@ $(OBJS_PL0E)

getSource.h : table.h
codegen.o : 	codegen.h getSource.h table.h
compile.o : 	codegen.h getSource.h table.h
getSource.o : getSource.h
pl0c_main.o : getSource.h y.tab.c
pl0e_main.o : codegen.h getSource.h
table.o : 		getSource.h
lex.yy.o :		lex.yy.c
y.tab.o :			y.tab.c

y.tab.c : pl0.y
	bison -d -y $<

lex.yy.c : pl0.l y.tab.c
	flex -l $<
