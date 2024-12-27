// FILE: test_dtag.c

#include "dtag.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_dtag_init() {
  uint8_t buffer[1024];
  dblock_t *block = NULL;
  int32_t result = dtag_init(&block, buffer, sizeof(buffer));
  assert(result == DTAG_OK);
  assert(block != NULL);
  assert(block->magic == DTAG_MAGIC);
  assert(block->version == DTAG_VERSION);
  assert(block->capacity == sizeof(buffer));
  assert(block->length == 0);
}

void test_dtag_import() {
  uint8_t buffer[1024];
  dblock_t *block = NULL;
  dtag_init(&block, buffer, sizeof(buffer));
  dtag_complete(block);

  dblock_t *imported_block = NULL;
  int32_t result = dtag_import(&imported_block, buffer, sizeof(buffer));
  assert(result == DTAG_OK);
  assert(imported_block != NULL);
  assert(imported_block->magic == DTAG_MAGIC);
  assert(imported_block->version == DTAG_VERSION);
}

void test_dtag_import_checksum_error() {
  uint8_t buffer[1024];
  dblock_t *block = NULL;
  dtag_init(&block, buffer, sizeof(buffer));
  dtag_complete(block);

  // Corrupt the checksum
  block->chksum[0] ^= 0xFF;

  dblock_t *imported_block = NULL;
  int32_t result = dtag_import(&imported_block, buffer, sizeof(buffer));
  assert(result == DTAG_ERR_CHECKSUM);
}

void test_dtag_complete() {
  uint8_t buffer[1024];
  dblock_t *block = NULL;
  dtag_init(&block, buffer, sizeof(buffer));
  int32_t result = dtag_complete(block);
  assert(result == DTAG_OK);
}

void test_dtag_get_set_del() {
  uint8_t buffer[1024];
  dblock_t *block = NULL;
  dtag_init(&block, buffer, sizeof(buffer));

  uint8_t value[] = {1, 2, 3, 4};
  int32_t result = dtag_set(block, DTAG_PCBASN, sizeof(value), value);
  assert(result == DTAG_OK);

  const ditem_t *item = dtag_get(block, DTAG_PCBASN);
  assert(item != NULL);
  assert(item->tag == DTAG_PCBASN);
  assert(item->len == sizeof(value));
  assert(memcmp(item->val, value, sizeof(value)) == 0);

  result = dtag_del(block, DTAG_PCBASN);
  assert(result == DTAG_OK);

  item = dtag_get(block, DTAG_PCBASN);
  assert(item == NULL);
}

int main() {
  test_dtag_init();
  test_dtag_import();
  test_dtag_import_checksum_error();
  test_dtag_complete();
  test_dtag_get_set_del();
  printf("All tests passed.\n");
  return 0;
}
