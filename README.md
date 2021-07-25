# macos-btrfs port

This project aims to port the BTRFS filesystem from the Linux kernel into a usable MacOS kernel extension, seamlessly integrated into diskutil. It's released under the GNU GPL v2, and much of it is based on the Apple ntfs extension source code, the Linux Kernel implementation of BTRFS, and the btrfs-progs project (all linked below).

Currently, only the btrfs.fs module is working, allowing macos to detect a BTRFS filesystem and its UUID (and stop annoying you about initializing it).

Apple Open Source NTFS Implementation: https://opensource.apple.com/source/ntfs/

BTRFS Wiki documentation: https://btrfs.wiki.kernel.org/index.php/On-disk_Format

BTRFS-Progs: https://github.com/kdave/btrfs-progs

Linux Kernel BTRFS Implementation: https://github.com/torvalds/linux/tree/master/fs/btrfs
