KMOD				= btrfs
SRCS				= vnode_if.h btrfs_kmod.c btrfs.c
MACHINE_ARCH		= amd64
MACHINE				= amd64
LDFLAGS				= -m elf_x86_64
CFLAGS				= --target=x86_64-freebsd -I../../btrfs/include
CC					= clang
LD					= ld.lld

.if ${.MAKE.OS} == "Darwin"
OBJCOPY				= llvm-objcopy
.elif ${.MAKE.OS} == "Linux"
# necessary because `xargs` on Linux doesn't support '-J', which is a FreeBSD extension
EXPORT_SYMS			= YES
OBJCOPY				= llvm-objcopy-14
.endif

.include <bsd.kmod.mk>
