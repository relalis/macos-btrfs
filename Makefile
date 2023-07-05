#
# All-in-one Makefile
#

MAKE=make
MV=mv
RM=rm
MKDIR=mkdir
OUT=bin

all: debug

debug:
	$(RM) -rf $(OUT)/macos_btrfs.kext* $(OUT)/mount_btrfs*
	$(MAKE) -C btrfs_filesystem $(TARGET)
	$(MAKE) -C mount $(TARGET)
	$(MKDIR) -p $(OUT)
	$(MV) btrfs_filesystem/macos_btrfs.kext btrfs_filesystem/macos_btrfs.kext.dSYM $(OUT)
	$(MV) mount/mount_btrfs $(OUT)
	$(MV) mount/mount_btrfs.dSYM $(OUT) 2> /dev/null || true

release: TARGET=release
release: debug

clean:
	$(RM) -rf $(OUT)/macos_btrfs.kext* $(OUT)/mount_btrfs*
	$(MAKE) -C btrfs_filesystem clean
	$(MAKE) -C mount clean

.PHONY: all debug release clean

