/*
 * Jonathan Doman
 * jonathan.doman@gmail.com
 */
#ifndef SHA256_H
#define SHA256_H

#include "Util.h"

#include <cstdint>
#include <vector>
#include <array>

class Sha256
{
public:
   struct Digest : public std::array<uint32_t,8>
   {
      Digest()
      {
         for( int i = 0; i < 8; ++i )
            at(i) = 0;
      }

      ByteArray toByteArray() const
      {
         ByteArray result( sizeof(*this) );
         for( unsigned i = 0; i < size(); ++i )
         {
            auto word = at( i );

            if( isBigEndian() )
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
   };

public:
   Sha256();
   ~Sha256();

   Sha256( const Sha256& other );

   void update( const void* data, int64_t bits );

   void digest( Digest& output );

private:
   void _hash( int blockIdx );

private:
   std::vector<uint8_t*> _msgBlocks;

   int64_t _msgBits;

   Digest _digest;
};

#endif // !SHA256_H
