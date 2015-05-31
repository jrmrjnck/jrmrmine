/**
 * This is free and unencumbered software released into the public domain.
**/

#include "Sha256.h"
#include "JsonRpc.h"
#include "Settings.h"
#include "Util.h"
#include "Transaction.h"
#include "Radix.h"
#include "Block.h"
#include "Miner.h"

#include <cassert>
#include <algorithm>
#include <cstdio>
#include <iostream>
#include <string>
#include <ctype.h>
#include <climits>
#include <chrono>
#include <thread>
#include <atomic>
#include <sstream>

using namespace std;

std::unique_ptr<Block> createBlockTemplate()
{
   JsonRpc rpc( Settings::RpcHost(), Settings::RpcPort(),
                Settings::RpcUser(), Settings::RpcPassword() );

   // Get block template
   Json::Value params;
   params[0u]["capabilities"] = Json::arrayValue;
   auto blockTemplate = rpc.call( "getblocktemplate", params );

   if( blockTemplate.isMember("coinbasetxn") )
      throw std::runtime_error( "Coinbase txn already exists" );

   // Get coinbase destination
   params.clear();
   params[0u] = "Mining Coinbase";
   auto coinbaseAddress = rpc.call( "getaccountaddress", params ).asString();
   // Convert address to pubkey hash
   auto coinbasePubKeyHash = Radix::base58DecodeCheck( coinbaseAddress );
   // Remove leading version byte
   coinbasePubKeyHash.erase( coinbasePubKeyHash.begin() );

   auto coinbaseValue = blockTemplate["coinbasevalue"].asInt64();

   // Create coinbase transaction
   auto coinbaseTxn = Transaction::createCoinbase( blockTemplate["height"].asInt(),
                                                   coinbaseValue,
                                                   coinbasePubKeyHash );

   // Create block
   std::unique_ptr<Block> block( new Block );
   block->header.version = blockTemplate["version"].asInt();
   block->header.time = blockTemplate["curtime"].asInt();
   block->header.bits = stoi( blockTemplate["bits"].asString(), nullptr, 16 );
   // Add all the transactions
   block->appendTransaction( coinbaseTxn );
   auto txnArray = blockTemplate["transactions"];
   assert( txnArray.isArray() );
   for( unsigned i = 0; i < txnArray.size(); ++i )
   {
      auto txnHash = hexStringToBinary( txnArray[i]["hash"].asString() );
      std::reverse( txnHash.begin(), txnHash.end() );
      block->appendTransactionHash( txnHash );
   }
   block->updateHeader();

   return block;
}

int main( int argc, char** argv )
{
   try
   {
      Settings::init( argc, argv );

      auto miner = Miner::createInstance( Settings::minerType() );
      if( miner == nullptr )
      {
         throw runtime_error( "Miner implementation doesn't exist" );
      }

      auto block = createBlockTemplate();

      if( miner->mine(*block) == Miner::SolutionFound )
      {
         std::cout << "Solution found: " << std::endl
                   << "\tHeader: " << block->headerData() << std::endl
                   << "\tHash:   " << Sha256::doubleHash( &block->header, sizeof(block->header) ) << std::endl;
      }
      else
      {
         std::cout << "No solution found" << std::endl;
      }

      return EXIT_SUCCESS;
   }
   catch( std::exception& e )
   {
      cerr << "ERROR: " << e.what() << endl;
      return EXIT_FAILURE;
   }
}
