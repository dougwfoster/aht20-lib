
CC           = gcc
SIZE         = size
SHLIB        = $(CC) -shared

SOVERSION    = 1

prefix ?= /usr/local
exec_prefix ?= $(prefix)
bindir ?= $(exec_prefix)/bin
includedir ?= $(prefix)/include
libdir ?= $(prefix)/lib

CFLAGS	+= -Wall -fpic

LIB_AHT20 = libaht20.so

OBJ_AHT20 = aht20.o

LIB = $(LIB_AHT20)

ALL = $(LIB)

LINK_AHT20  = -L. -laht20 -lrt

all: $(ALL)

aht20.o: source/AHT20.c source/AHT20.h
	$(CC) $(CFLAGS) -c -o aht20.o source/AHT20.c

clean:
	rm -f *.o *.s *~ $(ALL) *.so.$(SOVERSION)


install: $(ALL)
	@install -m 0755 -d                      $(DESTDIR)$(includedir)
	install -m 0644 source/AHT20.h                  $(DESTDIR)$(includedir)
	@install -m 0755 -d                      $(DESTDIR)$(libdir)
	install -m 0755 libaht20.so.$(SOVERSION) $(DESTDIR)$(libdir)
	@cd $(DESTDIR)$(libdir) && ln -fs libaht20.so.$(SOVERSION) libaht20.so
	@install -m 0755 -d                      $(DESTDIR)$(bindir)

ifeq ($(DESTDIR),)
	ldconfig
endif

uninstall:
	rm -f $(DESTDIR)$(includedir)/AHT20.h
	rm -f $(DESTDIR)$(libdir)/libaht20.so
	rm -f $(DESTDIR)$(libdir)/libaht20.so.$(SOVERSION)
ifeq ($(DESTDIR),)
	ldconfig
endif

$(LIB_AHT20):	$(OBJ_AHT20)
	$(SHLIB)  $(LDFLAGS) -Wl,-soname,$(LIB_AHT20).$(SOVERSION) -o $(LIB_AHT20).$(SOVERSION) $(OBJ_AHT20)
	ln -fs $(LIB_AHT20).$(SOVERSION) $(LIB_AHT20)
	$(SIZE)     $(LIB_AHT20)
