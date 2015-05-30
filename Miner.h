/*
 * Jonathan Doman
 * jonathan.doman@gmail.com
 */
#ifndef MINER_H
#define MINER_H

#include "Block.h"

#include <string>
#include <map>
#include <memory>
#include <vector>
#include <type_traits>

class Miner;
typedef std::unique_ptr<Miner> MinerPtr;

class Miner
{
public:
   typedef MinerPtr (*CreateInstanceFn)();

public:
   virtual ~Miner() = 0;

   bool mine( Block& block );

public:
   static MinerPtr createInstance( const std::string& typeName = std::string() );

   static void registerMinerType( const std::string& typeName, CreateInstanceFn fn );

   static std::vector<std::string> types();
};

template<typename T>
struct MinerRegistration
{
   static_assert( std::is_base_of<Miner,T>::value, "Registered type must be derived from Miner." );

   MinerRegistration( const char* alias )
   {
      Miner::registerMinerType( alias, T::createInstance );
   }
};

#endif // !MINER_H
