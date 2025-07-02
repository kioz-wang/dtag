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
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "dtag.h"
#include "logger/logger.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int32_t dtag_init(dblock_t **block, uint8_t *buf, uint32_t len) {
  if (len < sizeof(dblock_t)) {
    return DTAG_ERR_CAPACITY;
  }
  dblock_t *_block = (dblock_t *)buf;

  _block->magic = DTAG_MAGIC;
  _block->version = DTAG_VERSION;
  _block->chksum_length = CHKSUM_LENGTH;
  _block->capacity = len - sizeof(dblock_t);
  _block->length = 0;
  *block = _block;
  return DTAG_OK;
}

static int32_t _dtag_import_check0(const dblock_t *block) {
  if (block->magic != DTAG_MAGIC) {
    return DTAG_ERR_MAGIC;
  }
  if (block->version != DTAG_VERSION) {
    return DTAG_ERR_VERSION;
  }
  if (block->chksum_length != CHKSUM_LENGTH) {
    return DTAG_ERR_CHKSUM_LEN;
  }
  if (block->length > block->capacity) {
    return DTAG_ERR_LENGTH;
  }
  return DTAG_OK;
}

static int32_t _dtag_import_check1(const dblock_t *block, uint32_t len) {
  if (block->capacity > len - sizeof(dblock_t)) {
    return DTAG_ERR_CAPACITY;
  }
  return DTAG_OK;
}

static int32_t _dtag_import_final(dblock_t **block, uint8_t *buf) {
  dblock_t *_block = (dblock_t *)buf;
#if CHKSUM_LENGTH != 0
  uint8_t _chksum[CHKSUM_LENGTH];
  chksum_compute(_block->data, _block->length, _chksum);
  if (memcmp(_chksum, _block->chksum, CHKSUM_LENGTH) != 0) {
    return DTAG_ERR_CHECKSUM;
  }
#endif
  *block = _block;
  return DTAG_OK;
}

int32_t dtag_import(dblock_t **block, uint8_t *buf, uint32_t len) {
  if (len < sizeof(dblock_t)) {
    return DTAG_ERR_CAPACITY;
  }
  dblock_t *_block = (dblock_t *)buf;
  int32_t result = DTAG_OK;

  if (result == DTAG_OK)
    result = _dtag_import_check0(_block);
  if (result == DTAG_OK)
    result = _dtag_import_check1(_block, len);
  if (result == DTAG_OK)
    result = _dtag_import_final(block, buf);

  return result;
}

void dtag_complete(dblock_t *block) {
#if CHKSUM_LENGTH != 0
  chksum_compute(block->data, block->length, block->chksum);
#endif
}

int32_t dtag_import_file(dblock_t **block, const char *filename) {
  int32_t result = DTAG_OK;
  FILE *file = NULL;
  dblock_t _block;
  uint8_t *buf = NULL;

  if (result == DTAG_OK) {
    if (!(file = fopen(filename, "rb"))) {
      logfE("fail to open file: %s (%d:%s)", filename, errno, strerror(errno));
      result = DTAG_ERR_FILEIO;
    }
  }
  if (result == DTAG_OK) {
    if (fread(&_block, 1, sizeof(dblock_t), file) != sizeof(dblock_t)) {
      logfE("fail to read file: %s,%lu", filename, sizeof(dblock_t));
      result = DTAG_ERR_FILEIO;
    }
  }
  if (result == DTAG_OK) {
    result = _dtag_import_check0(&_block);
    if (result != DTAG_OK) {
      logfE("fail to check0 file: %s (%d)", filename, result);
    }
  }
  if (result == DTAG_OK) {
    if (!(buf = (uint8_t *)malloc(_block.capacity + sizeof(dblock_t)))) {
      logfE("fail to allocate memory: %lu", _block.capacity + sizeof(dblock_t));
      result = DTAG_ERR_NOMEM;
    }
  }
  if (result == DTAG_OK) {
    memcpy(buf, &_block, sizeof(dblock_t));
    if (fread(buf + sizeof(dblock_t), 1, _block.capacity, file) != _block.capacity) {
      logfE("fail to read file: %s,%d", filename, _block.capacity);
      result = DTAG_ERR_FILEIO;
    }
  }
  if (result == DTAG_OK) {
    result = _dtag_import_final(block, buf);
    if (result != DTAG_OK) {
      logfE("fail to final file: %s (%d)", filename, result);
    }
  }

  if (file) {
    fclose(file);
  }
  if (result != DTAG_OK && buf) {
    free(buf);
  }
  return result;
}

int32_t dtag_export_file(dblock_t *block, const char *filename) {
  FILE *file = NULL;
  uint32_t len = 0;

  file = fopen(filename, "wb");
  if (!file) {
    logfE("fail to open file: %s (%d:%s)", filename, errno, strerror(errno));
    return DTAG_ERR_FILEIO;
  }
  len = block->capacity + sizeof(dblock_t);
  if (fwrite(block, 1, len, file) != len) {
    logfE("fail to write file: %s,%d", filename, len);
    fclose(file);
    return DTAG_ERR_FILEIO;
  }
  fclose(file);
  return DTAG_OK;
}

inline static uint32_t _len(const ditem_t *item) { return sizeof(ditem_t) + item->klen + item->vlen; }
inline static ditem_t *_next(ditem_t *curr) { return (ditem_t *)((uint8_t *)curr + _len(curr)); }
inline static uint8_t *_begin(dblock_t *block) { return block->data; }
inline static uint8_t *_end(dblock_t *block) { return block->data + block->length; }

/**
 * @brief 检查地址合法性
 */
static int32_t _dtag_ditem_check0(dblock_t *block, ditem_t *item) {
  return _begin(block) <= (uint8_t *)item && (uint8_t *)item < _end(block);
}
/**
 * @brief 检查 `klen`, `vlen` 合法性
 * @note `item != _end(block)`
 */
static int32_t _dtag_ditem_check1(dblock_t *block, ditem_t *item) { return (uint8_t *)_next(item) <= _end(block); }

int32_t dtag_next(dblock_t *block, ditem_t **curr) {
  ditem_t *next = NULL;

  if (*curr) {
    if (!_dtag_ditem_check0(block, *curr) || !_dtag_ditem_check1(block, *curr)) {
      return DTAG_ERR_INVPARAM;
    }
    next = _next(*curr);
    if ((uint8_t *)next == _end(block)) {
      *curr = NULL;
      return 0;
    }
  } else {
    if (block->length == 0) {
      return 0;
    }
    next = (ditem_t *)_begin(block);
  }

  if (!_dtag_ditem_check1(block, next)) {
    return DTAG_ERR_DATA;
  }
  *curr = next;
  return 0;
}

/**
 * @brief
 *
 * @param block
 * @param key 会检查合法性（满足 `DTAG_MAX_KLEN`）
 * @param item 成功找到时，指向 `ditem`；可以传入 NULL，此时仅判定存在性
 * @return * int32_t 成功找到时，返回 DTAG_OK
 */
int32_t dtag_get_inner(dblock_t *block, const char *key, ditem_t **item) {
  if (strnlen(key, DTAG_MAX_KLEN) == DTAG_MAX_KLEN) {
    return DTAG_ERR_INVPARAM;
  }

  for (ditem_t *curr = NULL;;) {
    int32_t result = dtag_next(block, &curr);
    if (result != DTAG_OK)
      return result;
    if (curr == NULL)
      return DTAG_ERR_NOTFOUND;
    if (strnlen(key, DTAG_MAX_KLEN) != curr->klen - 1)
      continue;
    if (strcmp(key, (const char *)curr->kv))
      continue;
    if (item)
      *item = curr;
    break;
  }
  return DTAG_OK;
}

int32_t dtag_get(dblock_t *block, const char *key, uint8_t *val, uint32_t *len) {
  if (val && !len) {
    return DTAG_ERR_INVPARAM;
  }

  ditem_t *item = NULL;
  int32_t result = dtag_get_inner(block, key, &item);
  if (result != DTAG_OK)
    return result;

  if (val) {
    if (*len < item->vlen)
      return DTAG_ERR_NOSPACE;
    memcpy(val, &item->kv[item->klen], item->vlen);
  }
  if (len)
    *len = item->vlen;
  return DTAG_OK;
}

static void _dtag_del(dblock_t *block, ditem_t *item) {
  uint32_t len = _len(item);
  memmove((uint8_t *)item, (uint8_t *)_next(item), _end(block) - (uint8_t *)item - len);
  block->length -= len;
}

int32_t dtag_del(dblock_t *block, const char *key) {
  ditem_t *item = NULL;
  int32_t result = dtag_get_inner(block, key, &item);
  if (result != DTAG_OK)
    return result;
  _dtag_del(block, item);
  return DTAG_OK;
}

int32_t dtag_set(dblock_t *block, const char *key, const uint8_t *val, uint32_t len) {
  if (!val && len) {
    return DTAG_ERR_INVPARAM;
  }

  ditem_t *item = NULL;
  int32_t result = dtag_get_inner(block, key, &item);
  if (result != DTAG_OK && result != DTAG_ERR_NOTFOUND)
    return result;

  uint32_t klen = strnlen(key, DTAG_MAX_KLEN);
  if (item) {
    if (block->length - item->klen - item->vlen + klen + len > block->capacity) {
      return DTAG_ERR_CAPACITY;
    }
    _dtag_del(block, item);
  } else {
    if (block->length + sizeof(ditem_t) + klen + len > block->capacity) {
      return DTAG_ERR_CAPACITY;
    }
  }
  ditem_t *new_item = (ditem_t *)_end(block);
  new_item->klen = klen + 1;
  new_item->vlen = len;
  memcpy(new_item->kv, key, klen + 1);
  if (val) {
    memcpy(&new_item->kv[new_item->klen], val, len);
  }
  block->length += _len(new_item);
  return DTAG_OK;
}
