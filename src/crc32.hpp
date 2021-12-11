#ifndef _CRC32_H_
#define _CRC32_H_

#include <stddef.h>
#include <stdint.h>

uint32_t crc32c(uint32_t crc, const unsigned char *buf, size_t len);

#endif