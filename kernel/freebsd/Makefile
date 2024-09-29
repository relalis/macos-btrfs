KMOD				= btrfs
SRCS				= vnode_if.h btrfs.c btrfs_kmod.c
MACHINE_ARCH		= amd64
MACHINE				= amd64
LDFLAGS				= -m elf_x86_64
CFLAGS				= --target=x86_64-freebsd -I../include
CC					= clang

.if ${.MAKE.OS} == "Darwin"
LD					= ld.lld
OBJCOPY				= llvm-objcopy
.elif ${.MAKE.OS} == "Linux"
# `xargs` on Linux doesn't support '-J' (a FreeBSD extension), which results in error
EXPORT_SYMS			= YES
LD					= ld.lld-14
OBJCOPY				= llvm-objcopy-14
.endif

.include <bsd.kmod.mk>
