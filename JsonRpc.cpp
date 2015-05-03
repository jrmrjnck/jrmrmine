/*
 * Jonathan Doman
 * jonathan.doman@gmail.com
 */

#include "JsonRpc.h"

#include <curl/curl.h>

#include <stdexcept>
#include <cstring>
#include <cassert>
#include <thread>
#include <functional>
#include <iostream>

using namespace std;

// Adapted from bitcoin/src/util.cpp:EncodeBase64
static std::string encodeBase64( std::string str )
{
   const char s64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

   string ret;
   ret.reserve( (str.size() + 2) / 3 * 4 );

   int mode = 0;
   int left = 0;
   auto cur = str.data();
   auto end = cur + str.size();

   while( cur < end )
   {
      char enc = *(cur++);
      switch( mode )
      {
      case 0:
         ret += s64[enc >> 2];
         left = (enc & 0x3) << 4;
         mode = 1;
         break;

      case 1:
         ret += s64[left | (enc >> 4)];
         left = (enc & 0xF) << 2;
         mode = 2;
         break;

      case 2:
         ret += s64[left | (enc >> 6)];
         ret += s64[enc & 0x3F];
         mode = 0;
         break;
      }
   }

   if( mode != 0 )
   {
      ret += s64[left];
      ret += '=';
      if( mode == 1 )
         ret += '=';
   }

   return ret;
}

size_t recvPostData( char* ptr, size_t size, size_t nmemb, void* userdata )
{
   string& recvBuf = *reinterpret_cast<string*>(userdata);
   size_t bytes = size * nmemb;
   recvBuf.append( ptr, bytes );
   return bytes;
}

JsonRpc::JsonRpc( const std::string& url, 
                  int port,
                  const std::string& username, 
                  const std::string& password )
 : _headers(NULL)
{
   _url = url + ":" + to_string( port );

   // Encode RPC username and password
   string authPair;
   authPair += username + ':' + password;
   string authHeader( "Authorization: Basic " );
   authHeader += encodeBase64( authPair );

   _headers = curl_slist_append( _headers, authHeader.c_str() );
   _headers = curl_slist_append( _headers, "Content-Type: application/json" );
}

JsonRpc::~JsonRpc()
{
   curl_slist_free_all( _headers );
}

Json::Value JsonRpc::call( const std::string& method, const Json::Value& params )
{
   auto curl = curl_easy_init();
   if( curl == NULL )
      throw std::runtime_error( "Failed to initialize cURL" );

   Json::Value req;
   req["jsonrpc"] = "1.0";
   req["id"]      = static_cast<unsigned int>(hash<thread::id>()(this_thread::get_id()));
   req["method"]  = method;
   req["params"]  = params;

   cout << "REQUEST: " << endl << req << endl;

   Json::FastWriter writer;
   string data = writer.write( req );

   string recvData;

   curl_easy_setopt( curl, CURLOPT_URL, _url.c_str() );
   curl_easy_setopt( curl, CURLOPT_HTTPHEADER, _headers );
   curl_easy_setopt( curl, CURLOPT_POSTFIELDS, data.c_str() );
   curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, recvPostData );
   curl_easy_setopt( curl, CURLOPT_WRITEDATA, &recvData );

   auto code = curl_easy_perform( curl );
   if( code != CURLE_OK )
      throw runtime_error( curl_easy_strerror(code) );

   Json::Reader reader;
   Json::Value response;
   bool success = reader.parse( recvData, response );

   curl_easy_cleanup( curl );

   // Check the response
   if( recvData.size() == 0 )
      throw runtime_error( "No data received from server" );

   if( !success )
      throw runtime_error( reader.getFormatedErrorMessages() );

   if( !response.isObject() || !response.isMember("result") || !response.isMember("error") || !response.isMember("id") )
      throw runtime_error( "Invalid response received from JSON-RPC server" );

   if( !response["error"].isNull() )
      throw runtime_error( string("JSON-RPC Error: ") + response["error"]["message"].asString() );

   if( response["id"].asUInt() != req["id"].asUInt() )
      throw runtime_error( "Received response with wrong ID" );

   cout << "REPLY: " << endl << response << endl;

   return response["result"];
}
