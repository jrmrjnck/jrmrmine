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

   // Create block
   params.clear();
   params[0] = "2217e870bfd4fe5013628c7e6f1fcdc57abae74acca4c33ea45a8a88c405ba4d";
   auto prevBlockData = rpc.call( "getblock", params );
   Block prevBlock( prevBlockData["version"].asInt(),
                    prevBlockData["time"].asInt(),
                    stoi(prevBlockData["bits"].asString(),nullptr,16) );
   auto prevBlockHash = hexStringToBinary( prevBlockData["previousblockhash"].asString() );
   std::reverse( prevBlockHash.begin(), prevBlockHash.end() );
   prevBlock.setPrevBlockHash( prevBlockHash );
   auto txnArray = prevBlockData["tx"];
   assert( txnArray.isArray() );
   for( unsigned i = 0; i < txnArray.size(); ++i )
   {
      auto txnHash = hexStringToBinary( txnArray[i].asString() );
      std::reverse( txnHash.begin(), txnHash.end() );
      cout << txnHash << endl;
      prevBlock.appendTransactionHash( txnHash );
   }
   prevBlock.updateHeader();
   cout << prevBlock.merkleRoot() << endl;
   cout << Sha256::doubleHash( &prevBlock.header, sizeof(prevBlock.header) ) << endl;

   //Block block( blockTemplate["version"].asInt(),
                //blockTemplate["curtime"].asInt(),
                //stoi(blockTemplate["bits"].asString(),nullptr,16) );
   //// Add all the transactions
   //block.appendTransaction( coinbaseTxn );
   //auto txnArray = blockTemplate["transactions"];
   //assert( txnArray.isArray() );
   //for( unsigned i = 0; i < txnArray.size(); ++i )
   //{
      //auto txnHash = hexStringToBinary( txnArray[i]["hash"].asString() );
      //block.appendTransactionHash( txnHash );
   //}
   //block.update();
   //cout << block.merkleRoot() << endl;
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
