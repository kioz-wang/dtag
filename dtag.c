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
#include "logf/logf.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int32_t dtag_init(dblock_t **block, uint8_t *buf, uint32_t len) {
  if (len < sizeof(dblock_t)) {
    return DTAG_ERR_CAPACITY;
  }
  dblock_t *_block = (dblock_t *)buf;

  _block->magic = DTAG_MAGIC;
  _block->version = DTAG_VERSION;
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
  uint8_t _chksum[CHKSUM_LENGTH];

  chksum_compute(_block->data, _block->length, _chksum);
  if (memcmp(_chksum, _block->chksum, CHKSUM_LENGTH) != 0) {
    return DTAG_ERR_CHECKSUM;
  }
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
  chksum_compute(block->data, block->length, block->chksum);
}

int32_t dtag_import_file(dblock_t **block, const char *filename) {
  uint32_t filesize = 0;
  int32_t result = DTAG_OK;
  struct stat st;
  FILE *file = NULL;
  dblock_t _block;
  uint8_t *buf = NULL;

  if (result == DTAG_OK) {
    if (stat(filename, &st) != 0) {
      logfE("fail to stat file: %s", filename);
      result = DTAG_ERR_DATA;
    }
  }
  if (result == DTAG_OK) {
    if ((filesize = st.st_size) < sizeof(dblock_t)) {
      logfE("file size too small: %s", filename);
      result = DTAG_ERR_CAPACITY;
    }
  }
  if (result == DTAG_OK) {
    if (!(file = fopen(filename, "rb"))) {
      logfE("fail to open file: %s (%d:%s)", filename, errno, strerror(errno));
      result = DTAG_ERR_DATA;
    }
  }
  if (result == DTAG_OK) {
    if (fread(&_block, 1, sizeof(dblock_t), file) != sizeof(dblock_t)) {
      logfE("fail to read file: %s,%lu", filename, sizeof(dblock_t));
      result = DTAG_ERR_DATA;
    }
  }
  if (result == DTAG_OK) {
    result = _dtag_import_check0(&_block);
    if (result != DTAG_OK) {
      logfE("fail to check0 file: %s (%d)", filename, result);
    }
  }
  if (result == DTAG_OK) {
    result = _dtag_import_check1(&_block, filesize);
    if (result != DTAG_OK) {
      logfE("fail to check1 file: %s (%d)", filename, result);
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
    if (fread(buf + sizeof(dblock_t), 1, _block.capacity, file) !=
        _block.capacity) {
      logfE("fail to read file: %s,%d", filename, _block.capacity);
      result = DTAG_ERR_DATA;
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
    return DTAG_ERR_DATA;
  }
  len = block->capacity + sizeof(dblock_t);
  if (fwrite(block, 1, len, file) != len) {
    logfE("fail to write file: %s,%d", filename, len);
    fclose(file);
    return DTAG_ERR_DATA;
  }
  fclose(file);
  return DTAG_OK;
}

const ditem_t *dtag_get(dblock_t *block, dtag_t tag) {
  uint8_t *ptr = block->data;
  uint8_t *end = ptr + block->length;

  while (ptr < end) {
    ditem_t *item = (ditem_t *)ptr;
    if (item->tag == tag) {
      return item;
    }
    ptr += sizeof(ditem_t) + item->len;
  }
  return NULL;
}

static void _dtag_del(dblock_t *block, const ditem_t *item) {
  uint8_t *ptr = (uint8_t *)item;
  uint8_t *end = block->data + block->length;
  uint32_t len = sizeof(ditem_t) + item->len;

  memmove(ptr, ptr + len, end - ptr - len);
  block->length -= len;
}

int32_t dtag_del(dblock_t *block, dtag_t tag) {
  const ditem_t *item = dtag_get(block, tag);
  if (item == NULL) {
    return DTAG_ERR_NOTFOUND;
  }
  _dtag_del(block, item);
  return DTAG_OK;
}

int32_t dtag_set(dblock_t *block, dtag_t tag, uint32_t len, uint8_t *val) {
  const ditem_t *item = dtag_get(block, tag);
  if (item) {
    if (block->length - item->len + len > block->capacity) {
      return DTAG_ERR_CAPACITY;
    }
    _dtag_del(block, item);
  } else {
    if (block->length + sizeof(ditem_t) + len > block->capacity) {
      return DTAG_ERR_CAPACITY;
    }
  }
  ditem_t *new_item = (ditem_t *)(block->data + block->length);
  new_item->tag = tag;
  new_item->len = len;
  memcpy(new_item->val, val, len);
  block->length += sizeof(ditem_t) + len;
  return DTAG_OK;
}
