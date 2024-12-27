#include "token.h"

#include <assert.h>
#include <string.h>

int32_t line2tokens(char *line, const char *tokens[], uint32_t capa) {
  const char *opt = NULL;
  int32_t idx = 0;
  assert(capa >= 2);

  for (opt = strtok(line, " \n"); opt && idx < capa - 1;
       opt = strtok(NULL, " \n")) {
    tokens[idx++] = opt;
  }
  tokens[idx] = NULL;
  return idx;
}

void token_iter_init(token_iter_t *iter, const char *tokens[]) {
  iter->tokens = tokens;
  iter->i = 0;
}

int32_t fget_token_iter(token_iter_t *iter, char *linebuf, uint32_t size,
                        const char *tokens[], uint32_t capa, FILE *stream) {
  int32_t ret = 0;
  assert(capa >= 2);

  if (!fgets(linebuf, size, stream)) {
    return EOF;
  }
  ret = line2tokens(linebuf, tokens, capa);
  if (ret)
    token_iter_init(iter, tokens);
  return ret;
}

const char *token_iter_top(token_iter_t *iter) { return iter->tokens[iter->i]; }

const char *token_iter_pop(token_iter_t *iter) {
  const char *t = token_iter_top(iter);
  if (t) {
    iter->i++;
  }
  return t;
}

const char **token_iter_remain(token_iter_t *iter) {
  return &iter->tokens[iter->i];
}
