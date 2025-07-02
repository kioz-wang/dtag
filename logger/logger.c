#include "logger.h"
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FMT_MAX (1024)

static log_level_t g_log_level = LOG_DEBUG;

static void default_logger(const char *msg) { fputs(msg, stdout); }

static logger_f g_logger = default_logger;

static const char *log_level_names[LOG_DEBUG + 1] = {
    "ERROR", "WARNING", "INFO", "VERBOSE", "DEBUG",
};

static int32_t check_stderr_level() {
  static int32_t value = -2;
  const char *value_s = NULL;

  if (value == -2) {
#ifdef __LOGGER_ENV__
    value_s = getenv(__LOGGER_ENV__);
#endif /* __LOGGER_ENV__ */
    if (!value_s || value_s[0] == '\0') {
      value = -1;
    }
  }
  if (value != -2)
    return value;

  for (int32_t i = 0; i < LOG_DEBUG + 1; i++) {
    if (!strcmp(value_s, log_level_names[i])) {
      value = i;
      return value;
    }
  }

  char *endptr = NULL;
  unsigned long parsed = strtoul(value_s, &endptr, 0);
  if (value_s == endptr || parsed == ULONG_MAX) {
    value = LOG_DEBUG;
  } else {
    value = (parsed > LOG_DEBUG) ? LOG_DEBUG : (int32_t)parsed;
  }
  return value;
}

void logger(log_level_t lvl, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);

  int32_t stderr_level = check_stderr_level();

  if ((stderr_level >= 0 && lvl <= stderr_level) || lvl <= g_log_level) {
    char line[FMT_MAX] = {0};
    (void)vsnprintf(line, sizeof(line) - 1, fmt, args);

    if (stderr_level >= 0 && lvl <= stderr_level)
      fputs(line, stderr);
    if (lvl <= g_log_level)
      g_logger(line);
  }

  va_end(args);
}

void set_logger(log_level_t lvl, logger_f f) {
  g_log_level = (lvl > LOG_DEBUG) ? LOG_DEBUG : lvl;
  if (f)
    g_logger = f;
}

#ifdef __TEST_LOGGER__

int32_t main(int32_t argc, const char *argv[]) {
  set_logger(LOG_INFO, NULL);
  for (int32_t i = 0; i < LOG_DEBUG + 1; i++) {
    logger(i, "message level %s\n", log_level_names[i]);
  }
}

#endif /* __TEST_LOGGER__ */
