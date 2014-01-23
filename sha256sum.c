/*
 * Jonathan Doman
 * jonathan.doman@gmail.com
 *
 * Implementation based on http://csrc.nist.gov/groups/STM/cavp/documents/shs/sha256-384-512.pdf
 */

#include "sha256sum.h"

#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define ROTATE_RIGHT(x,n) ((x >> n) | (x << (sizeof(x)*CHAR_BIT-n)))

// The first 32 bits of the fractional parts of the cube roots of the first 64 primes
static const uint32_t K[] = {
   0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
   0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
   0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
   0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
   0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
   0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
   0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
   0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

int isBigEndian()
{
   union
   {
      uint32_t i;
      char c[4];
   } bint = {0x01020304};

   return bint.c[0] == 1;
}

void sha256sum( const uint8_t* message, int64_t bits, uint8_t* output )
{
   // Calculate padding
   int64_t totalBits = bits + 1 + 64;
   int64_t totalBlocks = totalBits / 512;
   int64_t padBits = 0;
   if( totalBits % 512 > 0 )
   {
      ++totalBlocks;
      padBits = totalBlocks*512 - totalBits;
   }
   int64_t totalBytes = totalBlocks * 64;

   assert( ((bits+1+padBits+64) % 512) == 0 );

   // Copy message to internal buffer
   uint8_t* msg = calloc( totalBytes, 1 );
   int64_t msgBytes = bits / 8;
   if( bits % CHAR_BIT > 0 )
      ++msgBytes;
   memcpy( msg, message, msgBytes );
   
   // Set 1 bit
   int byte = bits / 8;
   int pos = 7 - (bits % 8);
   msg[byte] |= (1 << pos);
   
   // Set message length
   uint64_t mask = 0xFF;
   int shift = 0;
   int b;
   for( b = 1; b <= 8; ++b )
   {
      msg[totalBytes-b] = (bits & mask) >> shift;
      mask <<= 8;
      shift += 8;
   }

   // Set initial hash values
   // Fractional parts of the square roots of the first 8 primes
   uint32_t hashValue[] = {
      0x6a09e667,
      0xbb67ae85,
      0x3c6ef372,
      0xa54ff53a,
      0x510e527f,
      0x9b05688c,
      0x1f83d9ab,
      0x5be0cd19
   };

   int i,j;
   for( i = 0; i < totalBlocks; ++i )
   {
      uint32_t a = hashValue[0];
      uint32_t b = hashValue[1];
      uint32_t c = hashValue[2];
      uint32_t d = hashValue[3];
      uint32_t e = hashValue[4];
      uint32_t f = hashValue[5];
      uint32_t g = hashValue[6];
      uint32_t h = hashValue[7];

      // Compute message schedule
      uint32_t w[64];
      // Copy first 16 words from the message block
      for( j = 0; j < 16; ++j )
      {
         int startM = i*64 + j*4;
         w[j]  = msg[startM+0] << 24;
         w[j] |= msg[startM+1] << 16;
         w[j] |= msg[startM+2] << 8;
         w[j] |= msg[startM+3];
      }
      // Compute the extended message
      for( j = 16; j < 64; ++j )
      {
         uint32_t s0 = ROTATE_RIGHT(w[j-15],7) ^ ROTATE_RIGHT(w[j-15],18) ^ (w[j-15] >> 3);
         uint32_t s1 = ROTATE_RIGHT(w[j-2],17) ^ ROTATE_RIGHT(w[j-2],19)  ^ (w[j-2] >> 10);
         w[j] = s1 + w[j-7] + s0 + w[j-16];
      }

      for( j = 0; j < 64; ++j )
      {
         uint32_t Ch  = (e & f) ^ (~e & g);
         uint32_t Maj = (a & b) ^ (a & c) ^ (b & c);
         uint32_t S0  = ROTATE_RIGHT(a,2) ^ ROTATE_RIGHT(a,13) ^ ROTATE_RIGHT(a,22);
         uint32_t S1  = ROTATE_RIGHT(e,6) ^ ROTATE_RIGHT(e,11) ^ ROTATE_RIGHT(e,25);

         uint32_t t1 = h + S1 + Ch + K[j] + w[j];
         uint32_t t2 = S0 + Maj;

         h = g;
         g = f;
         f = e;
         e = d + t1;
         d = c;
         c = b;
         b = a;
         a = t1 + t2;
      }

      hashValue[0] += a;
      hashValue[1] += b;
      hashValue[2] += c;
      hashValue[3] += d;
      hashValue[4] += e;
      hashValue[5] += f;
      hashValue[6] += g;
      hashValue[7] += h;
   }

   // Copy to output
   for( i = 0; i < 8; ++i )
   {
      if( isBigEndian() )
         *(uint32_t*)&output[i*4] = hashValue[i];
      else
      {
         output[i*4+0] = (hashValue[i] & 0xFF000000) >> 24;
         output[i*4+1] = (hashValue[i] & 0x00FF0000) >> 16;
         output[i*4+2] = (hashValue[i] & 0x0000FF00) >> 8;
         output[i*4+3] = (hashValue[i] & 0x000000FF) >> 0;
      }
   }

   free( msg );
}
