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
const int MSG_BLOCK_BYTES = MSG_BLOCK_BITS / CHAR_BIT;
const int MIN_PADDING_BITS = 1 + sizeof(int64_t) * CHAR_BIT;

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

#define ROTATE_RIGHT(x,n) ((x >> n) | (x << (sizeof(x)*CHAR_BIT-n)))

Sha256::Digest::Digest()
{
   fill( 0 );
}

ByteArray Sha256::Digest::toByteArray() const
{
   ByteArray result( sizeof(*this) );
   for( unsigned i = 0; i < size(); ++i )
   {
      auto word = at( i );

      if( !isLittleEndian() )
         *reinterpret_cast<uint32_t*>(&result[i * 4]) = word;
      else
      {
         result[i * 4 + 0] = (word) >> 24;
         result[i * 4 + 1] = (word & 0x00ff0000) >> 16;
         result[i * 4 + 2] = (word & 0x0000ff00) >> 8;
         result[i * 4 + 3] = (word & 0x000000ff);
      }
   }
   return result;
}

Sha256::Sha256()
{
   reset();
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
      _digest[i] = other._digest[i];
   }
}

Sha256::~Sha256()
{
   for( auto block : _msgBlocks )
      delete [] block;
}

void Sha256::reset()
{
   for( auto block : _msgBlocks )
      delete [] block;
   _msgBlocks.clear();

   _msgBits = 0;

   // Set initial hash values
   // Fractional parts of the square roots of the first 8 primes
   _digest[0] = 0x6a09e667;
   _digest[1] = 0xbb67ae85;
   _digest[2] = 0x3c6ef372;
   _digest[3] = 0xa54ff53a;
   _digest[4] = 0x510e527f;
   _digest[5] = 0x9b05688c;
   _digest[6] = 0x1f83d9ab;
   _digest[7] = 0x5be0cd19;
}

void Sha256::update( const void* data, int64_t bits )
{
   assert( (bits%8) == 0 && "Non-byte aligned update unsupported" );

   const uint8_t* input = reinterpret_cast<const uint8_t*>(data);
   uint8_t* output = NULL;

   int64_t bytesToCopy = bits / CHAR_BIT;
   int bitsInLastBlock = _msgBits % MSG_BLOCK_BITS;
   int availBytes = 0;
   if( bitsInLastBlock == 0 && _msgBits > 0 )
   {
      bitsInLastBlock = MSG_BLOCK_BITS;
   }

   if( bitsInLastBlock > 0 )
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
            _hash( _msgBlocks.back() );

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

void Sha256::_hash( const uint8_t* msg )
{
   uint32_t a = _digest[0];
   uint32_t b = _digest[1];
   uint32_t c = _digest[2];
   uint32_t d = _digest[3];
   uint32_t e = _digest[4];
   uint32_t f = _digest[5];
   uint32_t g = _digest[6];
   uint32_t h = _digest[7];

   // Compute message schedule
   uint32_t w[64];
   // Copy the first 16 words from the message block
   for( int j = 0; j < 16; ++j )
   {
      w[j]  = msg[j*4+0] << 8*3;
      w[j] |= msg[j*4+1] << 8*2;
      w[j] |= msg[j*4+2] << 8*1;
      w[j] |= msg[j*4+3] << 8*0;
   }

   // Compute the extended message
   for( int j = 16; j < 64; ++j )
   {
      uint32_t s0 = ROTATE_RIGHT(w[j-15],7) ^ ROTATE_RIGHT(w[j-15],18) ^ (w[j-15] >> 3);
      uint32_t s1 = ROTATE_RIGHT(w[j-2],17) ^ ROTATE_RIGHT(w[j-2],19)  ^ (w[j-2] >> 10);
      w[j] = s1 + w[j-7] + s0 + w[j-16];
   }

   // SHA-256 compression function, 64 rounds
#define S0(A) (ROTATE_RIGHT(A,2) ^ ROTATE_RIGHT(A,13) ^ ROTATE_RIGHT(A,22))
#define S1(E) (ROTATE_RIGHT(E,6) ^ ROTATE_RIGHT(E,11) ^ ROTATE_RIGHT(E,25))
#define CH(E,F,G) ((E & F) ^ (~E & G))
#define MAJ(A,B,C) ((A & B) ^ (A & C) ^ (B & C))

#define SHA_ROUND( A, B, C, D, E, F, G, H, N ) \
do { \
   t1 = H + S1(E) + CH(E,F,G) + K[N] + w[N]; \
   H = t1 + S0(A) + MAJ(A,B,C);\
   D = D + t1; \
} while( false )

#define SHA_ROUNDS_8( N ) \
do { \
   SHA_ROUND(a,b,c,d,e,f,g,h,N); \
   SHA_ROUND(h,a,b,c,d,e,f,g,N+1); \
   SHA_ROUND(g,h,a,b,c,d,e,f,N+2); \
   SHA_ROUND(f,g,h,a,b,c,d,e,N+3); \
   SHA_ROUND(e,f,g,h,a,b,c,d,N+4); \
   SHA_ROUND(d,e,f,g,h,a,b,c,N+5); \
   SHA_ROUND(c,d,e,f,g,h,a,b,N+6); \
   SHA_ROUND(b,c,d,e,f,g,h,a,N+7); \
} while( false )

   uint32_t t1;
   SHA_ROUNDS_8( 8*0 );
   SHA_ROUNDS_8( 8*1 );
   SHA_ROUNDS_8( 8*2 );
   SHA_ROUNDS_8( 8*3 );
   SHA_ROUNDS_8( 8*4 );
   SHA_ROUNDS_8( 8*5 );
   SHA_ROUNDS_8( 8*6 );
   SHA_ROUNDS_8( 8*7 );

#undef S0
#undef S1
#undef CH
#undef MAJ
#undef SHA_ROUND
#undef SHA_ROUNDS_8

   // Compute intermediate hash
   _digest[0] += a;
   _digest[1] += b;
   _digest[2] += c;
   _digest[3] += d;
   _digest[4] += e;
   _digest[5] += f;
   _digest[6] += g;
   _digest[7] += h;
}

void Sha256::digest( Digest& output )
{
   assert( (_msgBits%8) == 0 && "Non byte aligned usage unsupported" );

   // Check for space to insert padding and length
   int bitsInLastBlock = _msgBits % MSG_BLOCK_BITS;
   int availBits = MSG_BLOCK_BITS - bitsInLastBlock;
   if( _msgBits > 0 && bitsInLastBlock == 0 )
   {
      _hash( _msgBlocks.back() );
      availBits = 0;
      bitsInLastBlock = MSG_BLOCK_BITS;
   }

   uint8_t* pad = _msgBlocks.back() + (bitsInLastBlock / CHAR_BIT);
   if( availBits < MIN_PADDING_BITS )
   {
      _msgBlocks.push_back( new uint8_t[MSG_BLOCK_BYTES]() );
   }

   // Set following 1 bit
   if( availBits == 0 )
   {
      pad = _msgBlocks.back();
   }
   pad[0] = 0x80;

   // Set message length
   int64_t bits = _msgBits;
   pad = _msgBlocks.back() + MSG_BLOCK_BYTES - sizeof(bits);
   for( int i = sizeof(bits) - 1; i >= 0; --i )
   {
      pad[i] = bits & 0xff;
      bits >>= CHAR_BIT;
   }

   // The second to last block may have been modified
   if( availBits > 0 && availBits < MIN_PADDING_BITS )
      _hash( _msgBlocks.crbegin()[1] );

   // We know the last block was modified
   _hash( _msgBlocks.back() );

   // Copy to output
   for( int i = 0; i < 8; ++i )
   {
      output[i] = _digest[i];
   }
}

ByteArray Sha256::hash( const ByteArray& data )
{
   return hash( data.data(), data.size() );
}

ByteArray Sha256::hash( const void* data, int64_t bytes )
{
   Sha256 hash;
   Digest digest;

   hash.update( data, bytes * CHAR_BIT );
   hash.digest( digest );

   return digest.toByteArray();
}

ByteArray Sha256::doubleHash( const ByteArray& data )
{
   return doubleHash( data.data(), data.size() );
}

ByteArray Sha256::doubleHash( const void* data, int64_t bytes )
{
   return hash( hash( data, bytes ) );
}
