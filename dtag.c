#include "dtag.h"
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
  _block->capacity = len;
  _block->length = 0;
  *block = _block;
  return DTAG_OK;
}

int32_t dtag_import(dblock_t **block, uint8_t *buf, uint32_t len) {
  if (len < sizeof(dblock_t)) {
    return DTAG_ERR_CAPACITY;
  }
  dblock_t *_block = (dblock_t *)buf;
  if (_block->magic != DTAG_MAGIC) {
    return DTAG_ERR_MAGIC;
  }
  if (_block->version != DTAG_VERSION) {
    return DTAG_ERR_VERSION;
  }
  if (_block->length > len - sizeof(dblock_t)) {
    return DTAG_ERR_LENGTH;
  }
  if (_block->capacity > len) {
    return DTAG_WRAN_CAPACITY;
  }
  uint8_t _chksum[CHKSUM_LENGTH];
  chksum_compute(_block->data, _block->length, _chksum);
  if (memcmp(_chksum, _block->chksum, CHKSUM_LENGTH) != 0) {
    return DTAG_ERR_CHECKSUM;
  }
  *block = _block;
  return DTAG_OK;
}

int32_t dtag_complete(dblock_t *block) {
  chksum_compute(block->data, block->length, block->chksum);
  return DTAG_OK;
}

int32_t dtag_import_file(dblock_t **block, const char *filename) {
  FILE *file = fopen(filename, "rb");
  if (!file) {
    return DTAG_ERR_DATA;
  }
  fseek(file, 0, SEEK_END);
  uint32_t len = ftell(file);
  fseek(file, 0, SEEK_SET);
  uint8_t *buf = (uint8_t *)malloc(len);
  if (!buf) {
    fclose(file);
    return DTAG_ERR_NOMEM;
  }
  if (fread(buf, 1, len, file) != len) {
    fclose(file);
    free(buf);
    return DTAG_ERR_DATA;
  }
  fclose(file);
  int32_t result = dtag_import(block, buf, len);
  if (result != DTAG_OK) {
    free(buf);
  }
  return result;
}

int32_t dtag_export_file(dblock_t *block, const char *filename) {
  FILE *file = fopen(filename, "wb");
  if (!file) {
    return DTAG_ERR_DATA;
  }
  if (fwrite(block, 1, block->capacity, file) != block->capacity) {
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
    if (block->length + sizeof(dblock_t) - item->len + len > block->capacity) {
      return DTAG_ERR_CAPACITY;
    }
    _dtag_del(block, item);
  } else {
    if (block->length + sizeof(dblock_t) + sizeof(ditem_t) + len >
        block->capacity) {
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
