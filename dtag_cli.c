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
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <token/token.h>

#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE "\033[34m"
#define COLOR_CYAN "\033[36m"

void print_usage(const char *prog_name) {
  printf("Usage: %s <filename> <operation> [...]\n", prog_name);
  printf("Version %d:\n", DTAG_VERSION);
  printf("Operations:\n");
  printf("  init {capa}           - Initialize an empty file\n");
  printf("  dump                  - Dump the content of file\n");
  printf("  set {key} {value} ... - Set keys with the given value\n");
  printf("  get {key} ...         - Get the value of the given keys\n");
  printf("  setf {key} {file} ... - Set keys with the given files\n");
  printf("  getf {key} {file} ... - Get the given keys to files\n");
  printf("  del {key} ...         - Delete the given keys\n");
  printf("  hexdump               - Dump the content like hexdump -C\n");
}

inline static void print_error(const char *message) { logfE(COLOR_RED "%s" COLOR_RESET, message); }

inline static void print_info(const char *message) { logfI(COLOR_GREEN "%s" COLOR_RESET, message); }

int subcmd_init(const char *filename, const char *tokens[]) {
  token_iter_t it;
  token_iter_init(&it, tokens);
  const char *capa_str = token_iter_pop(&it);
  if (!capa_str) {
    print_error("Missing capacity");
    return EXIT_FAILURE;
  }
  uint32_t capc = strtoul(capa_str, NULL, 0);
  if (capc == 0 || capc > UINT32_MAX - sizeof(dblock_t)) {
    print_error("Invalid capacity");
    return EXIT_FAILURE;
  }
  uint8_t *buffer = (uint8_t *)malloc(capc + sizeof(dblock_t));
  if (!buffer) {
    print_error("Failed to allocate memory");
    return EXIT_FAILURE;
  }
  dblock_t *block = NULL;
  if (dtag_init(&block, buffer, capc + sizeof(dblock_t)) != DTAG_OK) {
    print_error("Failed to initialize dtag block");
    free(buffer);
    return EXIT_FAILURE;
  }
  dtag_complete(block);
  if (dtag_export_file(block, filename) != DTAG_OK) {
    print_error("Failed to export dtag block");
    free(buffer);
    return EXIT_FAILURE;
  }
  free(buffer);
  return EXIT_SUCCESS;
}

int subcmd_dump(const char *filename) {
  dblock_t *block = NULL;
  int32_t ret = dtag_import_file(&block, filename);
  if (ret != DTAG_OK) {
    print_error("Failed to import dtag block");
    return EXIT_FAILURE;
  }
  printf("Magic: %08x, Version: %u\n", block->magic, block->version);
  printf("Capacity: %u, Length: %u\n", block->capacity, block->length);
  printf("Chksum:");
  for (uint32_t i = 0; i < sizeof(block->chksum); i++) {
    printf(" %02x", block->chksum[i]);
  }
  printf("\n");
  for (ditem_t *curr = NULL;;) {
    int32_t result = dtag_next(block, &curr);
    if (result != DTAG_OK) {
      print_error("Failed to next");
      return EXIT_FAILURE;
    }
    if (curr == NULL)
      break;
    printf("Tag:%*s, Length: %u, Value: ", curr->klen, curr->kv, curr->vlen);
    for (uint32_t i = 0; i < curr->vlen; i++) {
      printf("%02x ", curr->kv[curr->klen + i]);
    }
    printf("\n");
  }
  free(block);
  return EXIT_SUCCESS;
}

int subcmd_set(const char *filename, const char *tokens[]) {
  dblock_t *block = NULL;
  int32_t ret = dtag_import_file(&block, filename);
  if (ret != DTAG_OK) {
    print_error("Failed to import dtag block");
    return EXIT_FAILURE;
  }
  token_iter_t it;
  token_iter_init(&it, tokens);
  while (token_iter_top(&it)) {
    const char *key_str = token_iter_pop(&it);
    const char *value_str = token_iter_pop(&it);
    if (!value_str) {
      print_error("Missing value");
      free(block);
      return EXIT_FAILURE;
    }
    uint32_t value_len = strlen(value_str) / 2;
    uint8_t *value = (uint8_t *)malloc(value_len);
    if (!value) {
      print_error("Failed to allocate memory");
      free(block);
      return EXIT_FAILURE;
    }
    for (uint32_t i = 0; i < value_len; i++) {
      char byte_str[3] = {value_str[i * 2], value_str[i * 2 + 1], '\0'};
      value[i] = strtoul(byte_str, NULL, 16);
    }
    if (dtag_set(block, key_str, value, value_len) != DTAG_OK) {
      print_error("Failed to set key");
      free(value);
      free(block);
      return EXIT_FAILURE;
    }
    free(value);
  }
  dtag_complete(block);
  if (dtag_export_file(block, filename) != DTAG_OK) {
    print_error("Failed to export dtag block");
    free(block);
    return EXIT_FAILURE;
  }
  free(block);
  return EXIT_SUCCESS;
}

int subcmd_get(const char *filename, const char *tokens[]) {
  dblock_t *block = NULL;
  int32_t ret = dtag_import_file(&block, filename);
  if (ret != DTAG_OK) {
    print_error("Failed to import dtag block");
    return EXIT_FAILURE;
  }
  token_iter_t it;
  token_iter_init(&it, tokens);
  while (token_iter_top(&it)) {
    ditem_t *item = NULL;
    const char *key_str = token_iter_pop(&it);
    ret = dtag_get_inner(block, key_str, &item);
    if (ret != DTAG_OK) {
      if (ret == DTAG_ERR_NOTFOUND) {
        print_error("Tag not found");
      } else {
        print_error("Failed to get_inner");
      }
      return EXIT_FAILURE;
    }
    printf("Tag:%*s, Length: %u, Value: ", item->klen, item->kv, item->vlen);
    for (uint32_t i = 0; i < item->vlen; i++) {
      printf("%02x ", item->kv[item->klen + i]);
    }
    printf("\n");
  }
  free(block);
  return EXIT_SUCCESS;
}

int subcmd_setf(const char *filename, const char *tokens[]) {
  dblock_t *block = NULL;
  int32_t ret = dtag_import_file(&block, filename);
  if (ret != DTAG_OK) {
    print_error("Failed to import dtag block");
    return EXIT_FAILURE;
  }
  token_iter_t it;
  token_iter_init(&it, tokens);
  while (token_iter_top(&it)) {
    const char *key_str = token_iter_pop(&it);
    const char *file = token_iter_pop(&it);
    if (!file) {
      print_error("Missing file");
      free(block);
      return EXIT_FAILURE;
    }
    FILE *f = fopen(file, "rb");
    if (!f) {
      print_error("Failed to open file");
      free(block);
      return EXIT_FAILURE;
    }
    fseek(f, 0, SEEK_END);
    uint32_t len = ftell(f);
    fseek(f, 0, SEEK_SET);
    uint8_t *value = (uint8_t *)malloc(len);
    if (!value) {
      print_error("Failed to allocate memory");
      fclose(f);
      free(block);
      return EXIT_FAILURE;
    }
    if (fread(value, 1, len, f) != len) {
      print_error("Failed to read file");
      fclose(f);
      free(value);
      free(block);
      return EXIT_FAILURE;
    }
    if (dtag_set(block, key_str, value, len) != DTAG_OK) {
      print_error("Failed to set key");
      fclose(f);
      free(value);
      free(block);
      return EXIT_FAILURE;
    }
    fclose(f);
    free(value);
  }
  dtag_complete(block);
  if (dtag_export_file(block, filename) != DTAG_OK) {
    print_error("Failed to export dtag block");
    free(block);
    return EXIT_FAILURE;
  }
  free(block);
  return EXIT_SUCCESS;
}

int subcmd_getf(const char *filename, const char *tokens[]) {
  dblock_t *block = NULL;
  int32_t ret = dtag_import_file(&block, filename);
  if (ret != DTAG_OK) {
    print_error("Failed to import dtag block");
    return EXIT_FAILURE;
  }
  token_iter_t it;
  token_iter_init(&it, tokens);
  while (token_iter_top(&it)) {
    ditem_t *item = NULL;
    const char *key_str = token_iter_pop(&it);
    const char *file = token_iter_pop(&it);
    if (!file) {
      print_error("Missing file");
      free(block);
      return EXIT_FAILURE;
    }
    ret = dtag_get_inner(block, key_str, &item);
    if (ret != DTAG_OK) {
      if (ret == DTAG_ERR_NOTFOUND) {
        print_error("Tag not found");
      } else {
        print_error("Failed to get_inner");
      }
      free(block);
      return EXIT_FAILURE;
    }
    FILE *f = fopen(file, "wb");
    if (!f) {
      print_error("Failed to open file");
      free(block);
      return EXIT_FAILURE;
    }
    if (fwrite(&item->kv[item->klen], 1, item->vlen, f) != item->vlen) {
      print_error("Failed to write file");
      fclose(f);
      free(block);
      return EXIT_FAILURE;
    }
    fclose(f);
  }
  free(block);
  return EXIT_SUCCESS;
}

int subcmd_del(const char *filename, const char *tokens[]) {
  dblock_t *block = NULL;
  int32_t ret = dtag_import_file(&block, filename);
  if (ret != DTAG_OK) {
    print_error("Failed to import dtag block");
    return EXIT_FAILURE;
  }
  token_iter_t it;
  token_iter_init(&it, tokens);
  while (token_iter_top(&it)) {
    const char *key_str = token_iter_pop(&it);
    if (!key_str) {
      print_error("Missing tag");
      free(block);
      return EXIT_FAILURE;
    }
    if (dtag_del(block, key_str) != DTAG_OK) {
      print_error("Failed to delete key");
      free(block);
      return EXIT_FAILURE;
    }
  }
  dtag_complete(block);
  if (dtag_export_file(block, filename) != DTAG_OK) {
    print_error("Failed to export dtag block");
    free(block);
    return EXIT_FAILURE;
  }
  free(block);
  return EXIT_SUCCESS;
}

int subcmd_hexdump(const char *filename) {
  dblock_t *block = NULL;
  int32_t ret = dtag_import_file(&block, filename);
  if (ret != DTAG_OK) {
    print_error("Failed to import dtag block");
    return EXIT_FAILURE;
  }

  uint8_t *ptr = (uint8_t *)block;
  uint32_t len = block->capacity + sizeof(dblock_t);
  int zero_line_count = 0;

  ditem_t *item = NULL;
  for (uint32_t i = 0; i < len; i += 16) {
    int is_zero_line = 1;
    for (uint32_t j = 0; j < 16 && i + j < len; j++) {
      if (ptr[i + j] != 0) {
        is_zero_line = 0;
        break;
      }
    }

    if (is_zero_line) {
      zero_line_count++;
      if (zero_line_count == 1) {
        printf("*\n");
      }
      continue;
    } else {
      zero_line_count = 0;
    }

    printf("%08x  ", i);
    for (uint32_t j = 0; j < 16; j++) {
      if (ptr + i + j < (uint8_t *)block->data + block->length) {
        if (i + j < sizeof(dblock_t)) {
          if (i + j < offsetof(dblock_t, magic) + sizeof(block->magic)) {
            printf(COLOR_CYAN "%02x " COLOR_RESET, ptr[i + j]);
          } else if (i + j < offsetof(dblock_t, version) + sizeof(block->version)) {
            printf(COLOR_GREEN "%02x " COLOR_RESET, ptr[i + j]);
          } else if (i + j < offsetof(dblock_t, chksum_length) + sizeof(block->chksum_length)) {
            printf(COLOR_RED "%02x " COLOR_RESET, ptr[i + j]);
          } else if (i + j < offsetof(dblock_t, capacity) + sizeof(block->capacity)) {
            printf(COLOR_YELLOW "%02x " COLOR_RESET, ptr[i + j]);
          } else if (i + j < offsetof(dblock_t, length) + sizeof(block->length)) {
            printf(COLOR_BLUE "%02x " COLOR_RESET, ptr[i + j]);
          } else if (i + j < offsetof(dblock_t, chksum) + sizeof(block->chksum)) {
            printf(COLOR_RED "%02x " COLOR_RESET, ptr[i + j]);
          } else {
            printf("%02x ", ptr[i + j]);
          }
        } else {
          if (item == NULL) {
            item = (ditem_t *)(block->data);
          } else if (ptr + i + j == (uint8_t *)item->kv + item->klen + item->vlen) {
            item = (ditem_t *)((uint8_t *)item->kv + item->klen + item->vlen);
          }
          if (ptr + i + j < (uint8_t *)item + 1) {
            printf(COLOR_CYAN "%02x " COLOR_RESET, ptr[i + j]);
          } else if (ptr + i + j < (uint8_t *)item + 4) {
            printf(COLOR_GREEN "%02x " COLOR_RESET, ptr[i + j]);
          } else if (ptr + i + j < (uint8_t *)item + offsetof(ditem_t, kv) + item->klen) {
            printf(COLOR_YELLOW "%02x " COLOR_RESET, ptr[i + j]);
          } else if (ptr + i + j < (uint8_t *)item + offsetof(ditem_t, kv) + item->klen + item->vlen) {
            printf(COLOR_BLUE "%02x " COLOR_RESET, ptr[i + j]);
          }
        }
      } else if (ptr + i + j < (uint8_t *)block->data + block->capacity) {
        printf("%02x ", ptr[i + j]);
      } else {
        printf("   ");
      }
    }
    printf(" |");
    for (uint32_t j = 0; j < 16; j++) {
      if (i + j < len) {
        if (ptr[i + j] >= 32 && ptr[i + j] <= 126) {
          printf("%c", ptr[i + j]);
        } else {
          printf(".");
        }
      }
    }
    printf("|\n");
  }

  free(block);
  return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
  if (argc < 3) {
    print_usage(argv[0]);
    return EXIT_FAILURE;
  }

  const char *filename = argv[1];
  const char *operation = argv[2];

  if (!strcmp(operation, "init")) {
    return subcmd_init(filename, (const char **)&argv[3]);
  }
  if (!strcmp(operation, "dump")) {
    return subcmd_dump(filename);
  }
  if (!strcmp(operation, "set")) {
    return subcmd_set(filename, (const char **)&argv[3]);
  }
  if (!strcmp(operation, "get")) {
    return subcmd_get(filename, (const char **)&argv[3]);
  }
  if (!strcmp(operation, "setf")) {
    return subcmd_setf(filename, (const char **)&argv[3]);
  }
  if (!strcmp(operation, "getf")) {
    return subcmd_getf(filename, (const char **)&argv[3]);
  }
  if (!strcmp(operation, "del")) {
    return subcmd_del(filename, (const char **)&argv[3]);
  }
  if (!strcmp(operation, "hexdump")) {
    return subcmd_hexdump(filename);
  }

  print_usage(argv[0]);
  return EXIT_FAILURE;
}
