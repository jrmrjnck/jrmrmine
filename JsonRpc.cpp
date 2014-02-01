/*
 * Jonathan Doman
 * jonathan.doman@gmail.com
 */

#include "JsonRpc.h"

#include <curl/curl.h>

#include <stdexcept>
#include <cstring>
#include <cassert>

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
   char** recvBuf = reinterpret_cast<char**>(userdata);
   size_t bytes = size * nmemb;
   *recvBuf = new char[bytes];
   memcpy( *recvBuf, ptr, bytes );
   return bytes;
}

JsonRpc::JsonRpc( const std::string& url, 
                  const std::string& username, 
                  const std::string& password )
 : _url(url),
   _headers(NULL)
{
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

Json::Value JsonRpc::call( const std::string& method, const Json::Value& json )
{
   auto curl = curl_easy_init();
   if( curl == NULL )
      throw std::runtime_error( "Failed to initialize cURL" );

   Json::Value copy( json );
   copy["method"] = method;
   Json::FastWriter writer;
   string data = writer.write( copy );

   char* recvData = NULL;

   curl_easy_setopt( curl, CURLOPT_URL, _url.c_str() );
   curl_easy_setopt( curl, CURLOPT_HTTPHEADER, _headers );
   curl_easy_setopt( curl, CURLOPT_POSTFIELDS, data.c_str() );
   curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, recvPostData );
   curl_easy_setopt( curl, CURLOPT_WRITEDATA, &recvData );

   curl_easy_perform( curl );

   Json::Reader reader;
   Json::Value response;
   reader.parse( recvData, recvData+strlen(recvData), response );
   delete [] recvData;

   curl_easy_cleanup( curl );

   // TODO: better error handling
   assert( response.isMember("result") );

   return response["result"];
}
