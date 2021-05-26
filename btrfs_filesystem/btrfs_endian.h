//
//  btrfs_endian.h
//  macos-btrfs
//
//  Created by Yehia Hafez on 5/24/21.
//

#ifndef btrfs_endian_h
#define btrfs_endian_h

#include <libkern/OSByteOrder.h>
#include "btrfs_types.h"

// Unigned endianness conversion functions

static inline u16 le16_to_cpu(le16 x)
{
	return (u16)(OSSwapLittleToHostInt16(x));
}

static inline u32 le32_to_cpu(le32 x)
{
	return (u32)(OSSwapLittleToHostInt32(x));
}

static inline u64 le64_to_cpu(le64 x)
{
	return (u64)(OSSwapLittleToHostInt64(x));
}

static inline u16 le16_to_cpup(le16 *x)
{
	return le16_to_cpu(*x);
}

static inline u32 le32_to_cpup(le32 *x)
{
	return le32_to_cpu(*x);
}

static inline u64 le64_to_cpup(le64 *x)
{
	return le64_to_cpu(*x);
}

static inline le16 cpu_to_le16(u16 x)
{
	return (le16)(OSSwapHostToLittleInt16(x));
}

static inline le32 cpu_to_le32(u32 x)
{
	return (le32)(OSSwapHostToLittleInt32(x));
}

static inline le64 cpu_to_le64(u64 x)
{
	return (le64)(OSSwapHostToLittleInt64(x));
}

static inline le16 cpu_to_le16p(u16 *x)
{
	return cpu_to_le16(*x);
}

static inline le32 cpu_to_le32p(u32 *x)
{
	return cpu_to_le32(*x);
}

static inline le64 cpu_to_le64p(u64 *x)
{
	return cpu_to_le64(*x);
}

// Signed endianness conversion functions

static inline s16 sle16_to_cpu(sle16 x)
{
	return (s16)le16_to_cpu((le16)x);
}

static inline s32 sle32_to_cpu(sle32 x)
{
	return (s32)le32_to_cpu((le32)x);
}

static inline s64 sle64_to_cpu(sle64 x)
{
	return (s64)le64_to_cpu((le64)x);
}

static inline s16 sle16_to_cpup(sle16 *x)
{
	return (s16)le16_to_cpu(*(le16*)x);
}

static inline s32 sle32_to_cpup(sle32 *x)
{
	return (s32)le32_to_cpu(*(le32*)x);
}

static inline s64 sle64_to_cpup(sle64 *x)
{
	return (s64)le64_to_cpu(*(le64*)x);
}

static inline sle16 cpu_to_sle16(s16 x)
{
	return (sle16)cpu_to_le16((u16)x);
}

static inline sle32 cpu_to_sle32(s32 x)
{
	return (sle32)cpu_to_le32((u32)x);
}

static inline sle64 cpu_to_sle64(s64 x)
{
	return (sle64)cpu_to_le64((u64)x);
}

static inline sle16 cpu_to_sle16p(s16 *x)
{
	return (sle16)cpu_to_le16(*(u16*)x);
}

static inline sle32 cpu_to_sle32p(s32 *x)
{
	return (sle32)cpu_to_le32(*(u32*)x);
}

static inline sle64 cpu_to_sle64p(s64 *x)
{
	return (sle64)cpu_to_le64(*(u64*)x);
}

#define const_le16_to_cpu(x) ((u16)(OSSwapLittleToHostConstInt16(((u16)(x)))))
#define const_le32_to_cpu(x) ((u32)(OSSwapLittleToHostConstInt32(((u32)(x)))))
#define const_le64_to_cpu(x) ((u64)(OSSwapLittleToHostConstInt64(((u64)(x)))))

#define const_cpu_to_le16(x) ((le16)(OSSwapHostToLittleConstInt16(((u16)(x)))))
#define const_cpu_to_le32(x) ((le32)(OSSwapHostToLittleConstInt32(((u32)(x)))))
#define const_cpu_to_le64(x) ((le64)(OSSwapHostToLittleConstInt64(((u64)(x)))))

#define const_sle16_to_cpu(x) ((s16)(OSSwapLittleToHostConstInt16(((u16)(x)))))
#define const_sle32_to_cpu(x) ((s32)(OSSwapLittleToHostConstInt32(((u32)(x)))))
#define const_sle64_to_cpu(x) ((s64)(OSSwapLittleToHostConstInt64(((u64)(x)))))

#define const_cpu_to_sle16(x)	\
		((sle16)(OSSwapHostToLittleConstInt16(((u16)(x)))))
#define const_cpu_to_sle32(x)	\
		((sle32)(OSSwapHostToLittleConstInt32(((u32)(x)))))
#define const_cpu_to_sle64(x)	\
		((sle64)(OSSwapHostToLittleConstInt64(((u64)(x)))))

#endif /* btrfs_endian_h */
