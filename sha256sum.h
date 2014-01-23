/*
 * Jonathan Doman
 * jonathan.doman@gmail.com
 */
#ifndef SHA256SUM_H
#define SHA256SUM_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

void sha256sum( const uint8_t* message, int64_t bits, uint8_t* output );

#ifdef __cplusplus
}
#endif

#endif // !SHA256SUM_H
