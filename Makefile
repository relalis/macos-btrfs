## This makefile is explicitly for GNU Make, and to cross-compile for FreeBSD
## You MUST define MAKESYSPATH below, or you can't build for BSD.

.PHONY: all clean bsd

export MAKESYSPATH=/home/yehia/extra_storage/freebsd_src/usr/src/share/mk

all:
	if [ -d "$(MAKESYSPATH)" ]; then bmake -C kernel/freebsd; fi
	$(MAKE) -C btrfs
	$(MAKE) -C test

clean:
	if [ -d "$(MAKESYSPATH)" ]; then bmake -C kernel/freebsd clean; fi
	$(MAKE) -C btrfs clean
	$(MAKE) -C test clean
