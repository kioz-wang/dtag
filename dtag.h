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
  DTAG_ERR_CHKSUM_LEN = -3,
  DTAG_ERR_CAPACITY = -4,
  DTAG_ERR_LENGTH = -5,
  DTAG_ERR_CHECKSUM = -6,
  DTAG_ERR_TAG = -7,
  DTAG_ERR_LEN = -8,
  DTAG_ERR_DATA = -9,
  DTAG_ERR_NOMEM = -10,
  DTAG_ERR_EXIST = -11,
  DTAG_ERR_NOTFOUND = -12,
  DTAG_ERR_FILEIO = -13,
  DTAG_ERR_INVPARAM = -14,
  DTAG_ERR_NOSPACE = -15,
};
typedef int32_t dtag_error_t;

struct dtag_item {
#define DTAG_MAX_KLEN (0x000000FF)
  /* `key` is null-terminaed string, `klen` includes null-terminator */
  uint32_t klen : 8;
#define DTAG_MAX_VLEN (0x00FFFFFF)
  /* `value` is byte array */
  uint32_t vlen : 24;
  uint8_t kv[];
} __attribute__((packed));
typedef struct dtag_item ditem_t;

struct dtag_block {
#define DTAG_MAGIC 0x44544147
  uint32_t magic;
#define DTAG_VERSION 0x03
  uint16_t version;
  // Fixed as CHKSUM_LENGTH
  uint16_t chksum_length;
  // The capacity of the data in bytes.
  uint32_t capacity;
  // The length of the data in bytes.
  uint32_t length;
  // The checksum of the data.
  uint8_t chksum[CHKSUM_LENGTH];
  uint8_t data[];
} __attribute__((packed));
typedef struct dtag_block dblock_t;

/**
 * @brief 在已知的 buffer 上划定 `len` 大小的连续区域作为 `dblock` 并初始化
 * 
 * @param block 返回 `dblock` 指针（实际指向 `buf` 的位置）
 * @param buf 
 * @param len 
 * @return * int32_t 
 */
extern int32_t dtag_init(dblock_t **block, uint8_t *buf, uint32_t len);
/**
 * @brief 在已知的 buffer 上取 `len` 大小的连续区域视为 `dblock` 并尝试解析
 * 
 * @param block 返回 `dblock` 指针（实际指向 `buf` 的位置）
 * @param buf 
 * @param len 
 * @return * int32_t 
 */
extern int32_t dtag_import(dblock_t **block, uint8_t *buf, uint32_t len);
/**
 * @brief 计算 `chksum`
 * 
 * @param block 
 * @return * void 
 */
extern void dtag_complete(dblock_t *block);

/**
 * @brief 从文件中读取数据并尝试解析为 `dblock`
 * 
 * @param block 返回 `dblock` 指针（需要用户释放）
 * @param filename 
 * @return * int32_t 
 */
extern int32_t dtag_import_file(dblock_t **block, const char *filename);
/**
 * @brief 将 `dblock` 完整写入到文件中
 * 
 * @param block 
 * @param filename 
 * @return * int32_t 
 */
extern int32_t dtag_export_file(dblock_t *block, const char *filename);

/**
 * @brief 获取下一个 `ditem`
 * @note 会检查当前和下一个 `ditem` 的合法性
 *
 * @param block
 * @param curr 传入当前的 `item`，会更新指向下一个；当传入 NULL 时，将返回首个；当不存在下一个时，会返回 NULL
 * @return * int32_t
 */
extern int32_t dtag_next(dblock_t *block, ditem_t **curr);
/**
 * @brief 获取 `ditem`
 *
 * @param block
 * @param key 会检查合法性（满足 `DTAG_MAX_KLEN`）
 * @param item 成功找到时，指向 `ditem`；可以传入 NULL，此时仅判定存在性
 * @return * int32_t 成功找到时，返回 DTAG_OK；未找到，返回 DTAG_ERR_NOTFOUND；以及其他错误
 */
extern int32_t dtag_get_inner(dblock_t *block, const char *key, ditem_t **item);
/**
 * @brief
 *
 * @param block
 * @param key
 * @param val 传入一个 buffer，用于存储 value；可以传入 NULL，此时仅查找，不取值
 * @param len 传入 buffer 的大小，同时返回 value 的大小；当 val 传入 NULL 时，可以传入 NULL
 * @return * int32_t 成功找到时，返回 DTAG_OK；未找到，返回 DTAG_ERR_NOTFOUND；以及其他错误
 */
extern int32_t dtag_get(dblock_t *block, const char *key, uint8_t *val, uint32_t *len);
extern int32_t dtag_del(dblock_t *block, const char *key);
/**
 * @brief
 *
 * @param block
 * @param key
 * @param val 传入一个 buffer，携带要写入的 value；可以传入 NULL，此时仍会设置 key
 * @param len 要写入的 value 的长度；当 val 为 NULL 时，长度应为 0
 * @return * int32_t
 */
extern int32_t dtag_set(dblock_t *block, const char *key, const uint8_t *val, uint32_t len);

#endif // __DTAG_H__
