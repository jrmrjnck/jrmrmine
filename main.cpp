/*
 * Jonathan Doman
 * jonathan.doman@gmail.com
 */

#include "Sha256.h"
#include "JsonRpc.h"
#include "Settings.h"
#include "Util.h"
#include "Transaction.h"

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
   auto& caps = params[0u]["capabilities"];
   caps.append( "coinbasetxn" );
   caps.append( "workid" );
   caps.append( "coinbase/append" );
   caps.append( "longpoll" );
   auto blockTemplate = rpc.call( "getblocktemplate", params );

   params.clear();
   params[0u] = "Mining Coinbase";
   // FIXME: Maybe call getnewaddress in the future?
   auto newAddressReply = rpc.call( "getaccountaddress", params );

   if( blockTemplate.isMember("coinbasetxn") )
      throw std::runtime_error( "Coinbase txn already exists" );

   // Hack to read 64-bit int from Json::Value, which doesn't provide a 64-bit
   // interface except in the unstable 0.6.0-rc
   stringstream ss;
   ss << blockTemplate["coinbasevalue"];
   double coinbaseValue = stod(ss.str()) / 100000000;
   cout << coinbaseValue << endl;

   // Create coinbase transaction
   params.clear();
   auto& cbInput = params[0u][0u];
   cbInput["txid"] = string( 64, '0' );
   cbInput["vout"] = 0;
   params[1][newAddressReply.asString()] = coinbaseValue;
   auto cbTxnReply = rpc.call( "createrawtransaction", params );

   // Insert word for extranonce
   auto cbTxnData = cbTxnReply.asString();
   hexStringToBinary( cbTxnData );
   cbTxnData[41] = 4;
   cbTxnData.insert( 42, 4, 0 );
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
