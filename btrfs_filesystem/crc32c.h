//
//  crc32c.h
//  macos-btrfs
//
//  Created by Yehia Hafez on 7/5/21.
//

#ifndef crc32c_h
#define crc32c_h

#include <stdint.h>

uint32_t calc_crc32c(uint32_t seed, uint8_t* msg, uint32_t msglen);

#endif /* crc32c_h */
