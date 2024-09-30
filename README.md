# macos-btrfs

This is a kernel extension aiming to provide simple read support for the [BTRFS](https://btrfs.readthedocs.io) filesystem.

It is *not* meant for production use, and may not be stable.

## Building
### Requirements
This project requires `clang`, `lld`, and `llvm-objcopy`. For FreeBSD, `bmake` is required.
### FreeBSD
By default, the Makefile builds a FreeBSD kernel module. Simply run `make` (using GNU make), and it will call
`bmake` on the FreeBSD kernel module subdirectory. The `MAKESYSPATH` variable in the project root directory
must be updated to point to the FreeBSD_source/usr/src/share/mk directory.

### MacOS
To build for macOS, simply run `make macos`. Cross-compiling is currently not supported for the macOS kernel.

## Background
While originally this project was aiming to port the Linux kernel implementation, and was released under the GNU
GPL v3, use of different sources became untenable and a rewrite was essential. This project is not exclusively
for macOS, but is heavily developed on FreeBSD due to the similarity between the two kernels, and the simplicity
of testing on FreeBSD (as opposed to spinning up a macOS vm for every new build). As such, this is released under
the terms of the BSD 2-clause license, but incorporates some source which has been released under BSD 4-clause
license. For the most part, this is original work for educational purposes, and is provided as-is, with no guarantee
of performance or functionality. Please feel free to read the LICENSE file for details on the BSD-2 license information.