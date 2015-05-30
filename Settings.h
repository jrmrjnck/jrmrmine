#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>

class Settings
{
public:
   static void init( int argc, char** argv );

   static bool debug();

   static std::string RpcHost();
   static int         RpcPort();
   static std::string RpcUser();
   static std::string RpcPassword();

   static const std::string& minerType();

   static std::string defaultConfigFile();
   static std::string minerTypes();
   static std::string typeHelpText();
};

#endif // !SETTINGS_H
