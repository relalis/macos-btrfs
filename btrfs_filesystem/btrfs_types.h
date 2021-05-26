//
//  btrfs_types.h
//  macos-btrfs
//
//  Created by Yehia Hafez on 5/24/21.
//

#ifndef btrfs_types_h
#define btrfs_types_h

#include <sys/types.h>
#include <mach/boolean.h>

/* Define our fixed size types. */
typedef u_int8_t u8;
typedef u_int16_t u16;
typedef u_int32_t u32;
typedef u_int64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

/*
 * Define our fixed size, little-endian types.  Note we define the signed types
 * to be unsigned so we do not get sign extension on endianness conversions.
 * We do not bother with eight-bit, little-endian types as endianness does not
 * apply for eight-bit types.
 */
typedef u_int16_t le16;
typedef u_int32_t le32;
typedef u_int64_t le64;
typedef u_int16_t sle16;
typedef u_int32_t sle32;
typedef u_int64_t sle64;

/*
 * Define our fixed size, big-endian types.  Note we define the signed types to
 * be unsigned so we do not get sign extension on endianness conversions.  We
 * do not bother with eight-bit, big-endian types as endianness does not apply
 * for eight-bit types.
 */
typedef u_int16_t be16;
typedef u_int32_t be32;
typedef u_int64_t be64;
typedef u_int16_t sbe16;
typedef u_int32_t sbe32;
typedef u_int64_t sbe64;

/* 2-byte Unicode character type. */
typedef le16 btrfschar;

#define BTRFS_SUPERBLOCK_SIZE 4096

#endif /* btrfs_types_h */
