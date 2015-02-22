CC=gcc
#CFLAGS=-O2 -pipe
CFLAGS=-Og -g
BUILD_CFLAGS=-std=gnu99 -I. -D_FILE_OFFSET_BITS=64 -pipe -Wall -pedantic
LDFLAGS=-lm

prefix=/usr
exec_prefix=${prefix}
bindir=${exec_prefix}/bin
mandir=${prefix}/man
datarootdir=${prefix}/share
datadir=${datarootdir}
sysconfdir=${prefix}/etc

all: calc manual

calc: calc.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(BUILD_CFLAGS) -o calc calc.o

manual:
#	gzip -9 < calc.1 > calc.1.gz

.c.o:
	$(CC) -c $(BUILD_CFLAGS) $(CFLAGS) $<

clean:
	rm -f *.o *~ calc debug.log *.?.gz

distclean:
	rm -f *.o *~ calc debug.log *.?.gz calc*.pkg.tar.*

install: all
#	install -D -o root -g root -m 0644 calc.1.gz $(DESTDIR)/$(mandir)/man1/calc.1.gz
#	install -D -o root -g root -m 0755 -s calc $(DESTDIR)/$(bindir)/calc

