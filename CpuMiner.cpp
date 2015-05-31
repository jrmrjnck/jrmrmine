/**
 * This is free and unencumbered software released into the public domain.
**/
#include "Miner.h"

#include <memory>
#include <limits>
#include <cassert>

class CpuMiner : public Miner
{
public:
   ~CpuMiner() {}

public:
   static MinerPtr createInstance()
   {
      return MinerPtr( new CpuMiner );
   }

protected:
   virtual Result _mine( const Sha256& preHash, const ByteArray& reverseTarget, uint32_t& nonce )
   {
      assert( reverseTarget.size() == sizeof(Sha256::Digest) );

      nonce = 0;
      do
      {
         // Complete the first hash
         Sha256 hash( preHash );
         Sha256::Digest digest;

         hash.update( &nonce, sizeof(nonce) * CHAR_BIT );
         hash.digest( digest );

         // Do it again
         auto result = Sha256::hash( digest.toByteArray() );

         for( int i = result.size() - 1; i >= 0; --i )
         {
            if( result[i] > reverseTarget[i] )
            {
               break;
            }

            if( result[i] < reverseTarget[i] )
            {
               return SolutionFound;
            }
         }
      } while( nonce++ < std::numeric_limits<uint32_t>::max() );

      return NoSolutionFound;
   }
};

MinerRegistration<CpuMiner> registration( "cpu" );
