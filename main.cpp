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

   Json::Value params;
   params[0u]["capabilities"] = Json::arrayValue;
   auto blockTemplate = rpc.call( "getblocktemplate", params );

   params.clear();
   params[0u] = "Mining Coinbase";
   auto newAddressReply = rpc.call( "getaccountaddress", params );

   if( blockTemplate.isMember("coinbasetxn") )
      throw std::runtime_error( "Coinbase txn already exists" );

   double coinbaseValue = blockTemplate["coinbasevalue"].asInt64();
   coinbaseValue /= SATOSHIS_PER_BITCOIN;

   // Create coinbase transaction
   params.clear();
   auto& cbInput = params[0u][0u];
   cbInput["txid"] = string( 64, '0' );
   cbInput["vout"] = 0;
   params[1][newAddressReply.asString()] = coinbaseValue;
   auto cbTxnReply = rpc.call( "createrawtransaction", params );

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
