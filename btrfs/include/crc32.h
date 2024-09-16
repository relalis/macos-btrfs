#ifndef _BSD_CRC32C_H
#define _BSD_CRC32C_H

#include <stdint.h>
#include <stddef.h>

extern const uint32_t crc32_tab[];

static __inline uint32_t
crc32_raw(const void *buf, size_t size, uint32_t crc)
{
	const uint8_t *p = (const uint8_t *)buf;

	while (size--)
		crc = crc32_tab[(crc ^ *p++) & 0xFF] ^ (crc >> 8);
	return (crc);
}

static __inline uint32_t
crc32(const void *buf, size_t size)
{
	uint32_t crc;

	crc = crc32_raw(buf, size, ~0U);
	return (crc ^ ~0U);
}

uint32_t
calculate_crc32c(uint32_t crc32c, const unsigned char *buffer,
    unsigned int length);


#endif // _BSD_CRC32_H