/*
 * Jonathan Doman
 * jonathan.doman@gmail.com
 */

#include "Sha256.h"
#include "JsonRpc.h"
#include "Settings.h"
#include "Util.h"

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
   auto blockTemplateReply = rpc.call( "getblocktemplate", params );

   params.clear();
   params[0u] = "Mining Coinbase";
   // FIXME: Maybe call getnewaddress in the future?
   auto newAddressReply = rpc.call( "getaccountaddress", params );

   if( blockTemplateReply.isMember("coinbasetxn") )
      throw std::runtime_error( "Coinbase txn already exists" );

   auto& coinbaseTxn = blockTemplateReply["coinbasetxn"];
   stringstream data;
   writeInt( data, 1 );
   writeVarInt( data, 1 );
   coinbaseTxn["data"] = data.str();
   cout << coinbaseTxn << endl;
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
