CC=gcc
CFLAGS=-std=c99 -Wall -Wextra
SOURCES= mount_btrfs.c
EXECUTABLE=mount_btrfs
RM=rm

all: debug

release: $(EXECUTABLE)

debug: CFLAGS += -g -DDEBUG -lutil
debug: release

$(EXECUTABLE): $(SOURCES)
	$(CC) $(CFLAGS) $< -o $@

clean:
	$(RM) -rf *.o $(EXECUTABLE) *.dSYM

.PHONY: all debug release clean

