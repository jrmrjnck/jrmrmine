#include "Block.h"

#include <sstream>
#include <cassert>
#include <cstring>

Block::Block()
{
   std::memset( &header, 0, sizeof(header) );
}

Block::Block( int version, int time, int bits )
 : Block()
{
   header.version = version;
   header.time = time;
   header.bits = bits;
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

ByteArray Block::headerData()
{
   ByteArray data;
   data.reserve( sizeof(header) );

   for( unsigned i = 0; i < sizeof(header); ++i )
   {
      data.push_back( reinterpret_cast<uint8_t*>(&header)[i] );
   }

   return data;
}
