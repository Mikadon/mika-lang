CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
INSTALL_DIR = /usr/local
BIN_DIR = $(INSTALL_DIR)/bin
INCLUDE_DIR = $(INSTALL_DIR)/include/mika

all: mika2c mikac

mika2c: mika2c.c
	$(CC) $(CFLAGS) -o mika2c mika2c.c

mikac: mikac.c
	$(CC) $(CFLAGS) -o mikac mikac.c

install: all
	mkdir -p $(BIN_DIR)
	mkdir -p $(INCLUDE_DIR)
	cp mika2c $(BIN_DIR)/
	cp mikac $(BIN_DIR)/
	cp mika_std.h $(INCLUDE_DIR)/
	cp mika_std.c $(INCLUDE_DIR)/
	chmod +x $(BIN_DIR)/mika2c
	chmod +x $(BIN_DIR)/mikac

uninstall:
	rm -f $(BIN_DIR)/mika2c
	rm -f $(BIN_DIR)/mikac
	rm -f $(INCLUDE_DIR)/mika_std.h
	rm -f $(INCLUDE_DIR)/mika_std.c

clean:
	rm -f mika2c mikac

test: all
	./mikac test.mk

.PHONY: all install uninstall clean test
