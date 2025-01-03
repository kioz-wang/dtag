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
 
#ifndef __LOGF_H__
#define __LOGF_H__

#include <stdint.h>
#include <stdarg.h>

enum logf_level {
  LOGF_DEBUG = 0,
  LOGF_INFO,
  LOGF_WARN,
  LOGF_ERROR,
  LOGF_FATAL,
};
typedef int32_t logf_level_t;

extern int32_t logf_default_logger(logf_level_t level, const char *module, const char *fmt, va_list args);

typedef typeof(logf_default_logger) logf_t;

extern void logf_set_level(logf_level_t level);

extern void logf_set_logger(logf_t *logger);

extern int32_t logf_(logf_level_t level, const char *module, const char *fmt, ...) __attribute__((format(printf, 3, 4)));

#define logfD(...) logf_(LOGF_DEBUG, __FILE__, __VA_ARGS__)
#define logfI(...) logf_(LOGF_INFO, __FILE__, __VA_ARGS__)
#define logfW(...) logf_(LOGF_WARN, __FILE__, __VA_ARGS__)
#define logfE(...) logf_(LOGF_ERROR, __FILE__, __VA_ARGS__)
#define logfF(...) logf_(LOGF_FATAL, __FILE__, __VA_ARGS__)

#endif // __LOGF_H__
