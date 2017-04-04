CC = cc
CFLAGS = -g -ansi -pedantic -Wall
LDFLAGS =
BISON = bison
BISON_FLAGS = -d -y
FLEX = flex
FLEX_FLAGS = -l

ifdef yacc
 yacc = 1
else
 yacc = 0
endif

OBJS_COMMON = codegen.o table.o
OBJS_PL0DC = $(OBJS_COMMON) pl0dcMain.o

ifeq ($(yacc), 0)
 OBJS_PL0DC += compile.o getSource.o
else
 OBJS_PL0DC += lex.yy.o
 CFLAGS += -DYACC
endif

OBJS_PL0DM = $(OBJS_COMMON) \
			getSource.o \
			pl0dmMain.o

.SUFFIXES	: .o .c

all : pl0dc pl0dm

install :
	cp -p pl0d[cm] /usr/bin

uninstall :
	rm -f /usr/bin/pl0d[cm]

clean	:
	rm -f pl0dc pl0dm *~ *.o lex.yy.c y.tab.*

.c.o :
	$(CC) $(CFLAGS) -c $<

pl0dc : $(OBJS_PL0DC)
	$(CC) -o $@ $(OBJS_PL0DC)

pl0dm : $(OBJS_PL0DM)
	$(CC) -o $@ $(OBJS_PL0DM)

getSource.h : table.h
codegen.o : codegen.h getSource.h table.h
compile.o : codegen.h getSource.h table.h
getSource.o : getSource.h
ifeq ($(yacc), 0)
 pl0dcMain.o : getSource.h
else
 pl0dcMain.o : getSource.h y.tab.c
endif
pl0dmMain.o : codegen.h getSource.h
table.o : getSource.h
lex.yy.o : lex.yy.c
y.tab.o : y.tab.c

y.tab.c : pl0d.y
	$(BISON) $(BISON_FLAGS) $<

lex.yy.c : pl0d.l y.tab.c
	$(FLEX) $(FLEX_FLAGS) $<
