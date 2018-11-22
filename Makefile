CC=gcc
CFLAGS=-O2 -pipe
#CFLAGS=-Og -g -pipe
BUILD_CFLAGS=-std=gnu99 -I. -D_FILE_OFFSET_BITS=64 -pipe -fstrict-aliasing
BUILD_CFLAGS += -Wall -Wextra -Wcast-align -Wstrict-aliasing -pedantic -Wstrict-overflow -Wno-unused-parameter
LDFLAGS=-lm

prefix=/usr
exec_prefix=${prefix}
bindir=${exec_prefix}/bin
mandir=${prefix}/man
datarootdir=${prefix}/share
datadir=${datarootdir}
sysconfdir=${prefix}/etc

BUILD_CFLAGS += $(CFLAGS_EXTRA)
ifdef DEBUG
BUILD_CFLAGS += -DDEBUG
endif

all: jodycalc manual

jodycalc: jodycalc.o
	$(CC) $(CFLAGS) $(LDFLAGS) $(BUILD_CFLAGS) -o jodycalc jodycalc.o

manual:
#	gzip -9 < jodycalc.1 > jodycalc.1.gz

.c.o:
	$(CC) -c $(BUILD_CFLAGS) $(CFLAGS) $<

clean:
	rm -f *.o *~ jodycalc debug.log *.?.gz

distclean:
	rm -f *.o *~ jodycalc debug.log *.?.gz jodycalc*.pkg.tar.*

install: all
#	install -D -o root -g root -m 0644 jodycalc.1.gz $(DESTDIR)/$(mandir)/man1/jodycalc.1.gz
#	install -D -o root -g root -m 0755 -s jodycalc $(DESTDIR)/$(bindir)/jodycalc

