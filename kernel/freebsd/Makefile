KMOD				= btrfs
SRCS				= vnode_if.h btrfs.c btrfs_kmod.c
MACHINE_ARCH		= amd64
MACHINE				= amd64
LDFLAGS				= -m elf_x86_64
CFLAGS				= --target=x86_64-freebsd -I../include
CC					= clang
LD					= ld.lld
OBJCOPY				= llvm-objcopy

.if ${.MAKE.OS} == "Linux"
# `xargs` on Linux doesn't support '-J' (a FreeBSD extension), which results in error
EXPORT_SYMS			= YES
# Debian clang 14.0.6 breaks fstack-protector, this is a temp workaround
CFLAGS				+= -DLINUX_CROSS_BUILD
.endif

.include <bsd.kmod.mk>
