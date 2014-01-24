/**
 * Jonathan Doman
 * jonathan.doman@gmail.com
 *
 * Implementation based on http://csrc.nist.gov/groups/STM/cavp/documents/shs/sha256-384-512.pdf
 */
#include "Sha256.h"

#include <algorithm>

#include <cassert>
#include <climits>
#include <cstring>

const int MSG_BLOCK_BITS = 512;
const int MSG_BLOCK_BYTES = MSG_BLOCK_BITS/CHAR_BIT;

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

template<typename T>
inline
T rotateRight( T x, int n )
{
   return ((x >> n) | (x << (sizeof(x)*CHAR_BIT-n)));
}

static int isBigEndian()
{
   union
   {
      uint32_t i;
      char c[4];
   } bint = {0x01020304};

   return bint.c[0] == 1;
}

Sha256::Sha256()
 : _msgBits(0)
{
   // Set initial hash values
   // Fractional parts of the square roots of the first 8 primes
   _hashVals[0] = 0x6a09e667;
   _hashVals[1] = 0xbb67ae85;
   _hashVals[2] = 0x3c6ef372;
   _hashVals[3] = 0xa54ff53a;
   _hashVals[4] = 0x510e527f;
   _hashVals[5] = 0x9b05688c;
   _hashVals[6] = 0x1f83d9ab;
   _hashVals[7] = 0x5be0cd19;
}

Sha256::Sha256( const Sha256& other )
{
   for( uint8_t* block : other._msgBlocks )
   {
      uint8_t* copy = new uint8_t[MSG_BLOCK_BYTES];
      memcpy( copy, block, MSG_BLOCK_BYTES );
      _msgBlocks.push_back( copy );
   }

   _msgBits = other._msgBits;

   for( int i = 0; i < 8; ++i )
   {
      _hashVals[i] = other._hashVals[i];
   }
}

Sha256::~Sha256()
{
   for( uint8_t* block : _msgBlocks )
      delete [] block;
}

void Sha256::update( const void* data, int64_t bits )
{
   assert( (bits%8) == 0 && "Non-byte aligned update unsupported" );

   const uint8_t* input = reinterpret_cast<const uint8_t*>(data);
   uint8_t* output = NULL;

   int64_t bytesToCopy = bits / CHAR_BIT;
   int bitsInLastBlock = _msgBits % MSG_BLOCK_BITS;
   int availBytes = 0;
   if( bitsInLastBlock != 0 )
   {
      assert( !_msgBlocks.empty() );
      availBytes = (MSG_BLOCK_BITS - bitsInLastBlock) / CHAR_BIT;
      output = _msgBlocks.back() + (bitsInLastBlock / CHAR_BIT);
   }

   while( bytesToCopy != 0 )
   {
      // Allocate a new block if we need to
      if( availBytes == 0 )
      {
         // Hash the one that was just filled, if applicable
         if( !_msgBlocks.empty() )
            _hash( _msgBlocks.size()-1 );

         _msgBlocks.push_back( new uint8_t[MSG_BLOCK_BYTES]() );
         availBytes = MSG_BLOCK_BYTES;
         output = _msgBlocks.back();
      }

      size_t bytes = std::min( bytesToCopy, static_cast<int64_t>(availBytes) );
      memcpy( output, input, bytes );

      bytesToCopy -= bytes;
      availBytes -= bytes;

      input += bytes;
      output += bytes;
   }

   _msgBits += bits;
}

void Sha256::_hash( int blockIdx )
{
   uint8_t* msg = _msgBlocks[blockIdx];

   uint32_t a = _hashVals[0];
   uint32_t b = _hashVals[1];
   uint32_t c = _hashVals[2];
   uint32_t d = _hashVals[3];
   uint32_t e = _hashVals[4];
   uint32_t f = _hashVals[5];
   uint32_t g = _hashVals[6];
   uint32_t h = _hashVals[7];

   // Compute message schedule
   uint32_t w[64];
   // Copy the first 16 words from the message block
   for( int j = 0; j < 16; ++j )
   {
      w[j]  = msg[j*4+0] << 24;
      w[j] |= msg[j*4+1] << 16;
      w[j] |= msg[j*4+2] << 8;
      w[j] |= msg[j*4+3];
   }

   // Compute the extended message
   for( int j = 16; j < 64; ++j )
   {
      uint32_t s0 = rotateRight(w[j-15],7) ^ rotateRight(w[j-15],18) ^ (w[j-15] >> 3);
      uint32_t s1 = rotateRight(w[j-2],17) ^ rotateRight(w[j-2],19)  ^ (w[j-2] >> 10);
      w[j] = s1 + w[j-7] + s0 + w[j-16];
   }

   // SHA-256 compression function, 64 rounds
   for( int j = 0; j < 64; ++j )
   {
      uint32_t Ch  = (e & f) ^ (~e & g);
      uint32_t Maj = (a & b) ^ (a & c) ^ (b & c);
      uint32_t S0  = rotateRight(a,2) ^ rotateRight(a,13) ^ rotateRight(a,22);
      uint32_t S1  = rotateRight(e,6) ^ rotateRight(e,11) ^ rotateRight(e,25);

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

   // Compute intermediate hash
   _hashVals[0] += a;
   _hashVals[1] += b;
   _hashVals[2] += c;
   _hashVals[3] += d;
   _hashVals[4] += e;
   _hashVals[5] += f;
   _hashVals[6] += g;
   _hashVals[7] += h;
}

void Sha256::digest( uint8_t* output )
{
   assert( (_msgBits%8) == 0 && "Non byte aligned usage unsupported" );

   // Check for space to insert padding and length
   int bitsInLastBlock = _msgBits % MSG_BLOCK_BITS;
   int availBits = MSG_BLOCK_BITS - bitsInLastBlock;
   uint8_t* pad = _msgBlocks.back() + (bitsInLastBlock / CHAR_BIT);
   if( availBits < (1 + 64) )
      _msgBlocks.push_back( new uint8_t[MSG_BLOCK_BYTES]() );

   // Set following 1
   if( availBits != 0 )
      pad[0] = 0x80;
   else
      _msgBlocks.back()[0] = 0x80;

   // Set message length
   int64_t bits = _msgBits;
   pad = _msgBlocks.back() + MSG_BLOCK_BYTES - sizeof(bits);
   for( int i = 0; i < sizeof(bits); ++i )
   {
      pad[i] = (bits >> (8*(7-i))) & 0xFF;
   }

   // The second to last block may have been modified
   if( availBits != 0 && availBits < (1 + 64) )
      _hash( _msgBlocks.size() - 2 );

   // We know the last block was modified
   _hash( _msgBlocks.size() - 1 );

   // Copy to output
   for( int i = 0; i < 8; ++i )
   {
      if( isBigEndian() )
         *reinterpret_cast<uint32_t*>(&output[i*4]) = _hashVals[i];
      else
      {
         output[i*4+0] = (_hashVals[i]) >> 24;
         output[i*4+1] = (_hashVals[i] & 0x00ff0000) >> 16;
         output[i*4+2] = (_hashVals[i] & 0x0000ff00) >> 8;
         output[i*4+3] = (_hashVals[i] & 0x000000ff);
      }
   }
}
