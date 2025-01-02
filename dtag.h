/*
 * MIT License
 * 
 * Copyright 2025 Kioz Wang <kioz.wang@gmail.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef __DTAG_H__
#define __DTAG_H__

#include "chksum/chksum.h"
#include <stdint.h>

enum dtag_error {
  DTAG_OK = 0,
  DTAG_ERR_MAGIC = -1,
  DTAG_ERR_VERSION = -2,
  DTAG_ERR_CAPACITY = -3,
  DTAG_ERR_LENGTH = -4,
  DTAG_ERR_CHECKSUM = -5,
  DTAG_ERR_TAG = -6,
  DTAG_ERR_LEN = -7,
  DTAG_ERR_DATA = -8,
  DTAG_ERR_NOMEM = -9,
  DTAG_ERR_EXIST = -10,
  DTAG_ERR_NOTFOUND = -11,
};
typedef int32_t dtag_error_t;

enum dtag {
  DTAG_PCBASN = 0x01,
  DTAG_CHALLENGE = 0x02,
  DTAG_XXXX,
  DTAG_MD5SUM = 0xFE,
};
typedef uint8_t dtag_t;

struct dtag_item {
  dtag_t tag;
  uint32_t len;
  uint8_t val[];
} __attribute__((packed));
typedef struct dtag_item ditem_t;

struct dtag_block {
#define DTAG_MAGIC 0x44544147
  uint32_t magic;
#define DTAG_VERSION 0x01
  uint32_t version;
  // The capacity of the data in bytes.
  uint32_t capacity;
  // The length of the data in bytes.
  uint32_t length;
  // The checksum of the data.
  uint8_t chksum[CHKSUM_LENGTH];
  uint8_t data[];
} __attribute__((packed));
typedef struct dtag_block dblock_t;

extern int32_t dtag_init(dblock_t **block, uint8_t *buf, uint32_t len);

extern int32_t dtag_import(dblock_t **block, uint8_t *buf, uint32_t len);

extern int32_t dtag_complete(dblock_t *block);

extern int32_t dtag_import_file(dblock_t **block, const char *filename);

extern int32_t dtag_export_file(dblock_t *block, const char *filename);

extern const ditem_t *dtag_get(dblock_t *block, dtag_t tag);

extern int32_t dtag_del(dblock_t *block, dtag_t tag);

extern int32_t dtag_set(dblock_t *block, dtag_t tag, uint32_t len,
                        uint8_t *val);

#endif // __DTAG_H__
