#include "dtag.h"
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
  printf("Operations:\n");
  printf("  init {capa}           - Initialize an empty file\n");
  printf("  dump                  - Dump the content of file\n");
  printf("  set {tag} {value} ... - Set tags with the given value\n");
  printf("  get {tag} ...         - Get the value of the given tags\n");
  printf("  del {tag} ...         - Delete the given tags\n");
  printf("  hexdump               - Dump the content like hexdump -C\n");
}

void print_error(const char *message) {
  fprintf(stderr, COLOR_RED "Error: %s" COLOR_RESET "\n", message);
}

void print_success(const char *message) {
  printf(COLOR_GREEN "%s" COLOR_RESET "\n", message);
}

void print_info(const char *message) {
  printf(COLOR_BLUE "%s" COLOR_RESET "\n", message);
}

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
  if (dtag_complete(block) != DTAG_OK) {
    print_error("Failed to complete dtag block");
    free(buffer);
    return EXIT_FAILURE;
  }
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
  const ditem_t *item = (const ditem_t *)(block->data);
  uint8_t *ptr = block->data;
  uint8_t *end = ptr + block->length;
  while (ptr < end) {
    printf("Tag: %02x, Length: %u, Value: ", item->tag, item->len);
    for (uint32_t i = 0; i < item->len; i++) {
      printf("%02x ", item->val[i]);
    }
    printf("\n");
    ptr += sizeof(ditem_t) + item->len;
    item = (const ditem_t *)ptr;
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
    const char *tag_str = token_iter_pop(&it);
    const char *value_str = token_iter_pop(&it);
    if (!tag_str || !value_str) {
      print_error("Missing tag or value");
      free(block);
      return EXIT_FAILURE;
    }
    dtag_t tag = strtoul(tag_str, NULL, 0);
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
    if (dtag_set(block, tag, value_len, value) != DTAG_OK) {
      print_error("Failed to set tag");
      free(value);
      free(block);
      return EXIT_FAILURE;
    }
    free(value);
  }
  if (dtag_complete(block) != DTAG_OK) {
    print_error("Failed to complete dtag block");
    free(block);
    return EXIT_FAILURE;
  }
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
    const char *tag_str = token_iter_pop(&it);
    if (!tag_str) {
      print_error("Missing tag");
      free(block);
      return EXIT_FAILURE;
    }
    dtag_t tag = strtoul(tag_str, NULL, 0);
    const ditem_t *item = dtag_get(block, tag);
    if (item) {
      printf("Tag: %02x, Length: %u, Value: ", item->tag, item->len);
      for (uint32_t i = 0; i < item->len; i++) {
        printf("%02x ", item->val[i]);
      }
      printf("\n");
    } else {
      print_error("Tag not found");
    }
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
    const char *tag_str = token_iter_pop(&it);
    if (!tag_str) {
      print_error("Missing tag");
      free(block);
      return EXIT_FAILURE;
    }
    dtag_t tag = strtoul(tag_str, NULL, 0);
    if (dtag_del(block, tag) != DTAG_OK) {
      print_error("Failed to delete tag");
      free(block);
      return EXIT_FAILURE;
    }
  }
  if (dtag_complete(block) != DTAG_OK) {
    print_error("Failed to complete dtag block");
    free(block);
    return EXIT_FAILURE;
  }
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
          } else if (i + j <
                     offsetof(dblock_t, version) + sizeof(block->version)) {
            printf(COLOR_GREEN "%02x " COLOR_RESET, ptr[i + j]);
          } else if (i + j <
                     offsetof(dblock_t, capacity) + sizeof(block->capacity)) {
            printf(COLOR_YELLOW "%02x " COLOR_RESET, ptr[i + j]);
          } else if (i + j <
                     offsetof(dblock_t, length) + sizeof(block->length)) {
            printf(COLOR_BLUE "%02x " COLOR_RESET, ptr[i + j]);
          } else if (i + j <
                     offsetof(dblock_t, chksum) + sizeof(block->chksum)) {
            printf(COLOR_RED "%02x " COLOR_RESET, ptr[i + j]);
          } else {
            printf("%02x ", ptr[i + j]);
          }
        } else {
          if (item == NULL) {
            item = (ditem_t *)(block->data);
          } else if (ptr + i + j == (uint8_t *)item->val + item->len) {
            item = (ditem_t *)((uint8_t *)item->val + item->len);
          }
          if (ptr + i + j <
              (uint8_t *)item + offsetof(ditem_t, tag) + sizeof(item->tag)) {
            printf(COLOR_CYAN "%02x " COLOR_RESET, ptr[i + j]);
          } else if (ptr + i + j < (uint8_t *)item + offsetof(ditem_t, len) +
                                       sizeof(item->len)) {
            printf(COLOR_GREEN "%02x " COLOR_RESET, ptr[i + j]);
          } else if (ptr + i + j <
                     (uint8_t *)item + offsetof(ditem_t, val) + item->len) {
            printf(COLOR_YELLOW "%02x " COLOR_RESET, ptr[i + j]);
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
  if (!strcmp(operation, "del")) {
    return subcmd_del(filename, (const char **)&argv[3]);
  }
  if (!strcmp(operation, "hexdump")) {
    return subcmd_hexdump(filename);
  }

  print_usage(argv[0]);
  return EXIT_FAILURE;
}
