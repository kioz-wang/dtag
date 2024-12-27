#ifndef __CHKSUM_H__
#define __CHKSUM_H__

#include <md5.h>
#include <stddef.h>

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#define CHKSUM_LENGTH (MD5_DIGEST_LENGTH)

extern void chksum_compute(const uint8_t *data, size_t length,
                              uint8_t chksum[CHKSUM_LENGTH]);

#endif /* __CHKSUM_H__ */
