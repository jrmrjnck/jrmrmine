/**
 * This is free and unencumbered software released into the public domain.
**/
#ifndef BLOCK_H
#define BLOCK_H

#include "Sha256.h"
#include "Transaction.h"
#include "MerkleTree.h"

#include <cstdint>

class Block
{
public:
#pragma pack(push,1)
   struct Header
   {
      uint32_t          version;
      Sha256::RawDigest prevBlock;
      Sha256::RawDigest merkleRoot;
      uint32_t          time;
      uint32_t          bits;
      uint32_t          nonce;
   };
#pragma pack(pop)

public:
   Block();
   Block( int version, int time, int bits );

   void setPrevBlockHash( const ByteArray& prevBlockHash );
   void appendTransaction( std::unique_ptr<Transaction> txn );

   void updateHeader();

   ByteArray headerData();
   ByteArray merkleRoot();

public:
   Header   header;

private:
   MerkleTree  _merkleTree;
   std::vector<std::unique_ptr<Transaction>> _txns;
};

#endif // !BLOCK_H
