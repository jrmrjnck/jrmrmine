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
   Transaction coinbaseTxn;
   coinbaseTxn.version = 1;
   coinbaseTxn.inputs.resize( 1 );
   auto& coinbaseInput = coinbaseTxn.inputs[0];
   coinbaseInput.prevHash.fill( 0 );
   coinbaseInput.prevN = -1;
   coinbaseInput.scriptSig << Script::Data(blockTemplate["height"].asInt(), 3) << 0 << 0 << 0 << 0;
   coinbaseInput.sequence = 0;
   coinbaseTxn.outputs.resize( 1 );
   auto& coinbaseOutput = coinbaseTxn.outputs[0];
   coinbaseOutput.value = coinbaseValue;
   coinbaseOutput.scriptPubKey << OP_DUP << OP_HASH160;
   coinbaseOutput.scriptPubKey << Script::Data(coinbasePubKeyHash.data(), coinbasePubKeyHash.size());
   coinbaseOutput.scriptPubKey << OP_EQUALVERIFY << OP_CHECKSIG;
   coinbaseTxn.storeSerial( std::cout );
   std::cout << std::endl;
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
