CC      = cc
CFLAGS  = -O2 -Wall
PREFIX  = /usr/local

SRC = src/main.c src/traverse.c src/xalloc.c src/readkey.c src/ccrypt.c \
      src/ccryptlib.c src/rijndael.c src/tables.c src/platform.c
OBJ = $(SRC:.c=.o)

cc: $(OBJ)
	$(CC) $(CFLAGS) -o cc $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

install: cc
	install -d $(DESTDIR)$(PREFIX)/bin
	install -m 755 cc $(DESTDIR)$(PREFIX)/bin/cc

clean:
	rm -f cc $(OBJ)

.PHONY: install clean
