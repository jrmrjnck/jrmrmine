/*
 * Jonathan Doman
 * jonathan.doman@gmail.com
 */

#ifndef JSONRPC_H
#define JSONRPC_H

#include <jsoncpp/json/json.h>

#include <string>

struct curl_slist;

class JsonRpc
{
public:
   JsonRpc( const std::string& url,
            int port,
            const std::string& username, 
            const std::string& password );
   ~JsonRpc();

   Json::Value call( const std::string& method, const Json::Value& json = Json::Value() );

private:
   std::string _url;
   curl_slist* _headers;
};

#endif // !JSONRPC_H
