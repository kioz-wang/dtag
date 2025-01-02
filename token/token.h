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

#ifndef __TOKEN_H__
#define __TOKEN_H__

#include <stdint.h>
#include <stdio.h>

typedef struct {
  const char **tokens;
  int32_t i;
} token_iter_t;

extern int32_t line2tokens(char *line, const char *tokens[], uint32_t capa);

extern void token_iter_init(token_iter_t *iter, const char *tokens[]);
extern int32_t fget_token_iter(token_iter_t *iter, char *linebuf, uint32_t size,
                               const char *tokens[], uint32_t capa,
                               FILE *stream);

extern const char *token_iter_top(token_iter_t *iter);
extern const char *token_iter_pop(token_iter_t *iter);
extern const char **token_iter_remain(token_iter_t *iter);

#endif /* __TOKEN_H__ */
