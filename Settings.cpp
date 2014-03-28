#include "Settings.h"

#include <boost/regex.hpp>

#include <map>
#include <fstream>
#include <cassert>
#include <regex>
#include <iostream>

using namespace std;

static std::map<std::string,std::string> _data;

void Settings::init( int argc, char** argv )
{
   // Load defaults
   _data["rpchost"] = "http://localhost";
   _data["rpcport"] = "18332";

   // Load default configuration file
   addConfigFile( defaultConfigFile() );

   // Load command line options
   (void)argc;
   (void)argv;
}

void Settings::addConfigFile( std::string filePath )
{
   ifstream configFile( filePath );
   if( !configFile.is_open() )
      return;

   string line;
   boost::regex keyValue( R"((\w+)\s*=\s*(\w+))" );      
   boost::smatch matches;
   while( !configFile.eof() )
   {
      getline( configFile, line );

      if( !boost::regex_match(line,matches,keyValue) )
         continue;

      _data[matches[1]] = matches[2];
   }
}

std::string Settings::RpcHost()
{
   return _data["rpchost"];
}

int Settings::RpcPort()
{
   return stoi( _data["rpcport"] );
}

std::string Settings::RpcUser()
{
   return _data["rpcuser"];
}

std::string Settings::RpcPassword()
{
   return _data["rpcpassword"];
}
