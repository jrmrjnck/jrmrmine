#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>

class Settings
{
public:
   static void init( int argc, char** argv );
   static void addConfigFile( std::string filePath );

   static std::string RpcHost();
   static int         RpcPort();
   static std::string RpcUser();
   static std::string RpcPassword();

   static std::string defaultConfigFile()
   {
      std::string home = getenv( "HOME" );
      return home + "/.bitcoin/bitcoin.conf";
   }
};

#endif // !SETTINGS_H
