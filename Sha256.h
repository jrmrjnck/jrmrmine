/*
 * Jonathan Doman
 * jonathan.doman@gmail.com
 */
#ifndef SHA256_H
#define SHA256_H

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
