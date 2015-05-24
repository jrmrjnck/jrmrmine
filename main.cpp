/*
 * Jonathan Doman
 * jonathan.doman@gmail.com
 */

#include "Sha256.h"
#include "JsonRpc.h"
#include "Settings.h"
#include "Util.h"
#include "Transaction.h"
#include "Radix.h"
#include "Block.h"

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

void getBlockTemplate()
{
   JsonRpc rpc( Settings::RpcHost(), Settings::RpcPort(),
                Settings::RpcUser(), Settings::RpcPassword() );

   // Get block template
   Json::Value params;
   params[0u]["capabilities"] = Json::arrayValue;
   auto blockTemplate = rpc.call( "getblocktemplate", params );

   // Get coinbase destination
   params.clear();
   params[0u] = "Mining Coinbase";
   auto coinbaseAddress = rpc.call( "getaccountaddress", params ).asString();
   // Convert address to pubkey hash
   auto coinbasePubKeyHash = Radix::base58DecodeCheck( coinbaseAddress );
   // Remove leading version byte
   coinbasePubKeyHash.erase( coinbasePubKeyHash.begin() );

   if( blockTemplate.isMember("coinbasetxn") )
      throw std::runtime_error( "Coinbase txn already exists" );

   auto coinbaseValue = blockTemplate["coinbasevalue"].asInt64();

   // Create coinbase transaction
   auto coinbaseTxn = Transaction::createCoinbase( blockTemplate["height"].asInt(),
                                                   coinbaseValue,
                                                   coinbasePubKeyHash );
}

int main( int argc, char** argv )
{
   try
   {
      Settings::init( argc, argv );

      getBlockTemplate();
   }
   catch( std::exception& e )
   {
      cerr << "ERROR: " << e.what() << endl;
   }

   return 0;
}
