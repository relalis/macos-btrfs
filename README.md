# macos-btrfs port

This work-in-progress project aims to port the [BTRFS filesystem](https://btrfs.readthedocs.io/en/latest/dev/On-disk-format.html) from the Linux kernel into a usable MacOS kernel extension, seamlessly integrated into diskutil. It's released under the GNU GPL v3, and much of it is based on the [Apple ntfs extension](https://opensource.apple.com/source/ntfs/) source code, the [Linux Kernel implementation of BTRFS](https://github.com/torvalds/linux/tree/master/fs/btrfs), and [WinBtrfs](https://github.com/maharmstone/btrfs).

Currently, this does not mount, as the vfs_mount functions haven't been implemented yet; as a small side project, development on this progresses in turtle years -- and only when I have free time (which tends to be never). If you'd like to contribute, please feel free to submit a PR.
