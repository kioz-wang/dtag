#ifndef __LOGGER_H__
#define __LOGGER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

enum log_level {
  LOG_ERROR,
  LOG_WARNING,
  LOG_INFO,
  LOG_VERBOSE,
  LOG_DEBUG,
};
typedef uint8_t log_level_t;

typedef void (*logger_f)(const char *msg);

void logger(log_level_t lvl, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));
void set_logger(log_level_t lvl, logger_f f);

#define logfE(fmt, ...) logger(LOG_ERROR, fmt, ##__VA_ARGS__)
#define logfW(fmt, ...) logger(LOG_WARNING, fmt, ##__VA_ARGS__)
#define logfI(fmt, ...) logger(LOG_INFO, fmt, ##__VA_ARGS__)
#define logfV(fmt, ...) logger(LOG_VERBOSE, fmt, ##__VA_ARGS__)
#define logfD(fmt, ...) logger(LOG_DEBUG, fmt, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* __LOGGER_H__ */
