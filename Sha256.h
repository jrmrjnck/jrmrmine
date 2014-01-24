/*
 * Jonathan Doman
 * jonathan.doman@gmail.com
 */
#ifndef SHA256_H
#define SHA256_H

#include <cstdint>
#include <vector>

class Sha256
{
public:
   Sha256();
   ~Sha256();

   Sha256( const Sha256& other );

   void update( const void* data, int64_t bits );

   void digest( uint8_t* output );

private:
   void _hash( int blockIdx );

private:
   std::vector<uint8_t*> _msgBlocks;

   int64_t _msgBits;

   uint32_t _hashVals[8];
};

#endif // !SHA256_H
