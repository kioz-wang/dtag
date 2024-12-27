#include "chksum.h"

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

void chksum_compute(const uint8_t *data, size_t length,
                       uint8_t chksum[CHKSUM_LENGTH]) {
  MD5_CTX ctx;
  MD5Init(&ctx);
  MD5Update(&ctx, data, length);
  MD5Final(chksum, &ctx);
}
