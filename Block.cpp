#include "Block.h"

#include <sstream>
#include <cassert>

Block::Block( int version, int time, int bits )
{
   header.version = version;
   header.time = time;
   header.bits = bits;
   header.nonce = 0;
}

void Block::setPrevBlockHash( const ByteArray& prevBlockHash )
{
   assert( prevBlockHash.size() == sizeof(header.prevBlock) );
   std::copy( prevBlockHash.begin(), prevBlockHash.end(), header.prevBlock.begin() );
}

void Block::appendTransaction( const Transaction& txn )
{
   appendTransactionHash( txn.id() );
}

void Block::appendTransactionHash( const ByteArray& txnId )
{
   _merkleTree.append( txnId );
}

void Block::updateHeader()
{
   auto merkleRoot = _merkleTree.rootHash();
   assert( merkleRoot.size() == sizeof(header.merkleRoot) );
   std::copy( merkleRoot.begin(), merkleRoot.end(), header.merkleRoot.begin() );
}

ByteArray Block::merkleRoot()
{
   return _merkleTree.rootHash();
}
