## This makefile is explicitly for GNU Make, and to cross-compile for FreeBSD
## You MUST define MAKESYSPATH below, or you can't build for BSD.

.PHONY: all clean macos

export MAKESYSPATH=/home/yehia/extra_storage/freebsd_src/usr/src/share/mk

all:
	if [ -d "$(MAKESYSPATH)" ]; then bmake -C kernel/freebsd; fi
	$(MAKE) -C test

clean:
	if [ -d "$(MAKESYSPATH)" ]; then bmake -C kernel/freebsd clean; fi
	$(MAKE) -C test clean

macos:
	$(MAKE) -C kernel/macos
	$(MAKE) -C tools