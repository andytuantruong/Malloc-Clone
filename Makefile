#
# R Jesse Chaney
# 

CC = gcc
DEBUG = -g
DEFINES =

CFLAGS = $(DEBUG) -Wall -Wshadow -Wunreachable-code -Wredundant-decls \
        -Wmissing-declarations -Wold-style-definition -Wmissing-prototypes \
        -Wdeclaration-after-statement -Wunsafe-loop-optimizations $(DEFINES)
PROG = vikalloc


all: $(PROG)


vikalloc: vikalloc.o main.o
	$(CC) $(CFLAGS) -o $@ $^
	chmod a+rx,g-w $@

vikalloc.o: vikalloc.c vikalloc.h Makefile
	$(CC) $(CFLAGS) -c $<

main.o: main.c vikalloc.h Makefile
	$(CC) $(CFLAGS) -c $<

opt: clean
	make DEBUG=-O3

tar canvas: clean
	tar cvfz ${LOGNAME}_$(PROG).tar.gz *.[ch] ?akefile

# clean up the compiled files and editor chaff
clean cls:
	rm -f $(PROG) *.o *~ \#*

ci:
	if [ ! -d RCS ] ; then mkdir RCS; fi
	ci -t-none -m"lazy-checkin" -l *.[ch] ?akefile *.bash

gi:
	if [ ! -d RCS ] ; then git init; fi
	git add *.[ch] ?akefile
	git commit *.[ch] ?akefile
