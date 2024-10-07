KMOD				= btrfs
SRCS				= vnode_if.h btrfs.c btrfs_tree.c btrfs_kmod.c
MACHINE_ARCH		= amd64
MACHINE				= amd64
LDFLAGS				= -m elf_x86_64
CFLAGS				= --target=x86_64-freebsd -I../include
CC					= clang-14
LD					= ld.lld-14
OBJCOPY				= llvm-objcopy-14

.if ${.MAKE.OS} == "Linux"
# `xargs` on Linux doesn't support '-J' (a FreeBSD extension), which results in error
EXPORT_SYMS			= YES
# Debian clang 14.0.6 breaks fstack-protector, this is a temp workaround
# __stack_chk_guard is just breaking randomly on clang. Adding O2 for some
# absurd reason fixes linking.
CFLAGS				+= -DLINUX_CROSS_BUILD -O2
.endif
.include <bsd.kmod.mk>
