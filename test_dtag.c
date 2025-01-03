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
  assert(block->capacity == sizeof(buffer) - sizeof(dblock_t));
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
  test_dtag_get_set_del();
  printf("All tests passed.\n");
  return 0;
}
