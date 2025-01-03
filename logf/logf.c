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

#include "logf.h"
#include <stdio.h>

static logf_level_t logf_level = LOGF_INFO;
static logf_t *logf_logger = NULL;

int32_t logf_default_logger(logf_level_t level, const char *module,
                            const char *fmt, va_list args) {
  const char *level_str = NULL;
  int32_t len = 0;
  switch (level) {
  case LOGF_DEBUG:
    level_str = "DEBUG";
    break;
  case LOGF_INFO:
    level_str = "INFO";
    break;
  case LOGF_WARN:
    level_str = "WARN";
    break;
  case LOGF_ERROR:
    level_str = "ERROR";
    break;
  case LOGF_FATAL:
    level_str = "FATAL";
    break;
  default:
    level_str = "UNKNOWN";
    break;
  }

  len += fprintf(stderr, "[%s] %s: ", level_str, module);
  len += vfprintf(stderr, fmt, args);
  len += fprintf(stderr, "\n");
  return len;
}

void logf_set_level(logf_level_t level) { logf_level = level; }

void logf_set_logger(logf_t *logger) { logf_logger = logger; }

int32_t logf_(logf_level_t level, const char *module, const char *fmt, ...) {
  if (level < logf_level) {
    return 0;
  }
  logf_t *_logf_logger = logf_logger;

  if (!_logf_logger) {
    _logf_logger = logf_default_logger;
  }
  va_list args;
  va_start(args, fmt);
  int32_t ret = _logf_logger(level, module, fmt, args);
  va_end(args);
  return ret;
}
