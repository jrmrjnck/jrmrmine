/**
 * This is free and unencumbered software released into the public domain.
**/

#include "Miner.h"

#include <cstddef>
#include <algorithm>

struct  MinerRegistry
{
   static MinerRegistry& get()
   {
      static MinerRegistry* registry = new MinerRegistry;
      return *registry;
   }

   std::map<std::string, Miner::CreateInstanceFn> types;
};

Miner::~Miner()
{
}

MinerPtr Miner::createInstance( const std::string& typeName )
{
   auto& types = MinerRegistry::get().types;

   auto fn = types.find( typeName );

   if( fn == types.end() )
   {
      return nullptr;
   }

   return fn->second();
}

void Miner::registerMinerType( const std::string& typeName, CreateInstanceFn fn )
{
   MinerRegistry::get().types[typeName] = fn;
}

std::vector<std::string> Miner::types()
{
   std::vector<std::string> result;

   for( auto it : MinerRegistry::get().types )
   {
      result.push_back( it.first );
   }

   return result;
}

Miner::Result Miner::mine( Block& block )
{
   // Precompute as much hash as possible
   Sha256 hash;
   hash.update( &block.header, offsetof(Block::Header, nonce) * CHAR_BIT );

   auto target = bitsToTarget( block.header.bits );
   std::reverse( target.begin(), target.end() );

   auto result = _mine( hash, target, block.header.nonce );

   return result;
}
