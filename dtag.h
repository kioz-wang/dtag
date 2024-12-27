#ifndef __DTAG_H__
#define __DTAG_H__

#include "chksum/chksum.h"
#include <stdint.h>

enum dtag_error {
  DTAG_OK = 0,
  DTAG_WRAN_CAPACITY = 1,
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
  // The capacity of the block in bytes.
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
