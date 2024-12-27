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
