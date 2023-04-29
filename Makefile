
LEX = flex
LFLAGS =

YACC = bison
YFLAGS = -d

CC = gcc
CFLAGS = -Wall

as: lex.yy.c risc.tab.c risc_common.c
	$(CC) $(CFLAGS) $^ -o $@ -lfl

lex.yy.c: risc.l
	$(LEX) $(LFAGS) $^

risc.tab.c risc.tab.h: risc.y
	$(YACC) $(YFLAGS) $^

clean:
	$(RM) a.out as lex.yy.c risc.tab.c risc.tab.h

.PHONY: clean
