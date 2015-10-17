/**
 * This is free and unencumbered software released into the public domain.
**/

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

#define OPT_HELP     "help"
#define OPT_DEBUG    "debug"
#define OPT_CONFIG   "config"
#define OPT_TYPE     "type"
#define OPT_BLOCKS   "blocks"

#define OPT_RPCHOST     "rpchost"
#define OPT_RPCPORT     "rpcport"
#define OPT_RPCPASSWORD "rpcpassword"
#define OPT_RPCUSER     "rpcuser"

void Settings::init( int argc, char** argv )
{
   BoostProgOpt::options_description generalOptions( "General Options" );
   generalOptions.add_options()
      (OPT_HELP",h",    "Print this help.")
      (OPT_DEBUG",d",   "Show debug output.")
      (OPT_CONFIG",c",  BoostProgOpt::value<string>()->default_value(defaultConfigFile()), "Bitcoin Core configuration file to load.")
      (OPT_TYPE",t",    BoostProgOpt::value<string>()->default_value("cpu"), typeHelpText().c_str())
      (OPT_BLOCKS",n",  BoostProgOpt::value<int>()->default_value(0), "Number of blocks to mine (0 = unlimited).")
      ;

   BoostProgOpt::options_description coreOptions( "Bitcoin Core Options" );
   coreOptions.add_options()
      (OPT_RPCHOST,     BoostProgOpt::value<string>()->default_value("http://localhost"))
      (OPT_RPCPORT,     BoostProgOpt::value<int>()->default_value(18332))
      (OPT_RPCPASSWORD, BoostProgOpt::value<string>())
      (OPT_RPCUSER,     BoostProgOpt::value<string>())
      ;

   BoostProgOpt::options_description allOptions;
   allOptions.add(generalOptions).add(coreOptions);

   // Read all recognized options from command line
   BoostProgOpt::store( BoostProgOpt::parse_command_line(argc,argv,allOptions), _varMap );
   BoostProgOpt::notify( _varMap );

   if( _varMap.count(OPT_HELP) )
   {
      cout << allOptions;
      exit( 0 );
   }

   // Read settings from configuration file, ignoring unknown settings
   const auto& configFile = _varMap[OPT_CONFIG].as<string>();
   BoostProgOpt::store( BoostProgOpt::parse_config_file<char>(configFile.c_str(),coreOptions,true), _varMap );
   BoostProgOpt::notify( _varMap );
}

std::string Settings::RpcHost()
{
   return _varMap[OPT_RPCHOST].as<string>();
}

int Settings::RpcPort()
{
   return _varMap[OPT_RPCPORT].as<int>();
}

std::string Settings::RpcUser()
{
   return _varMap[OPT_RPCUSER].as<string>();
}

std::string Settings::RpcPassword()
{
   return _varMap[OPT_RPCPASSWORD].as<string>();
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
   return _varMap[OPT_TYPE].as<std::string>();
}

bool Settings::debug()
{
   return _varMap.count( OPT_DEBUG );
}

int Settings::numBlocks()
{
   return _varMap[OPT_BLOCKS].as<int>();
}
