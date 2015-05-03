#include "Settings.h"

#include <boost/program_options.hpp>

#include <map>
#include <fstream>
#include <cassert>
#include <iostream>

using namespace std;
namespace BoostProgOpt = boost::program_options;

static BoostProgOpt::variables_map _varMap;

void Settings::init( int argc, char** argv )
{
   BoostProgOpt::options_description generalOptions( "General Options" );
   generalOptions.add_options()
      ("help,h", "Print this help.")
      ("config,c", BoostProgOpt::value<string>()->default_value(defaultConfigFile()), "Bitcoin Core configuration file to load.")
      ;

   BoostProgOpt::options_description coreOptions( "Bitcoin Core Options" );
   coreOptions.add_options()
      ("rpchost", BoostProgOpt::value<string>()->default_value("http://localhost"))
      ("rpcport", BoostProgOpt::value<int>()->default_value(18332))
      ("rpcpassword", BoostProgOpt::value<string>())
      ("rpcuser", BoostProgOpt::value<string>())
      ;

   BoostProgOpt::options_description allOptions;
   allOptions.add(generalOptions).add(coreOptions);

   // Read all recognized options from command line
   BoostProgOpt::store( BoostProgOpt::parse_command_line(argc,argv,allOptions), _varMap );
   BoostProgOpt::notify( _varMap );

   if( _varMap.count("help") )
   {
      cout << allOptions;
      exit( 0 );
   }

   // Read settings from configuration file, ignoring unknown settings
   const auto& configFile = _varMap["config"].as<string>();
   BoostProgOpt::store( BoostProgOpt::parse_config_file<char>(configFile.c_str(),coreOptions,true), _varMap );
   BoostProgOpt::notify( _varMap );
}

std::string Settings::RpcHost()
{
   return _varMap["rpchost"].as<string>();
}

int Settings::RpcPort()
{
   return _varMap["rpcport"].as<int>();
}

std::string Settings::RpcUser()
{
   return _varMap["rpcuser"].as<string>();
}

std::string Settings::RpcPassword()
{
   return _varMap["rpcpassword"].as<string>();
}
