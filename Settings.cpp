#include "Settings.h"
#include "Miner.h"

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
      ("debug,d", "Show debug output.")
      ("config,c", BoostProgOpt::value<string>()->default_value(defaultConfigFile()), "Bitcoin Core configuration file to load.")
      ("type,t", BoostProgOpt::value<string>()->default_value("cpu"), typeHelpText().c_str())
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

std::string Settings::defaultConfigFile()
{
   std::string home = getenv( "HOME" );
   return home + "/.bitcoin/bitcoin.conf";
}

std::string Settings::typeHelpText()
{
   auto types = Miner::types();

   if( types.size() < 1 )
   {
      throw std::runtime_error( "No kernel implementations are registered" );
   }

   std::string helpText( "Hashing kernel implementation type. Possible values are" );

   for( auto& typeName : types )
   {
      helpText.append( " \"" ).append( typeName ).append( "\"," );
   }
   helpText.pop_back();
   helpText.append( "." );

   return helpText;
}

const std::string& Settings::minerType()
{
   return _varMap["type"].as<std::string>();
}

bool Settings::debug()
{
   return _varMap.count( "debug" );
}
