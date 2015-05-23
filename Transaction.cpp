#include "Transaction.h"

#include <sstream>
#include <cassert>
#include <iomanip>

using std::string;
using std::stringstream;

void TxnInput::loadSerial( std::istream& serialStream )
{
   // Load the outpoint TXID
   string hashStr( sizeof(Sha256::Digest) * 2, 0 );
   serialStream.read( &hashStr[0], hashStr.size() );
   ByteArray binaryTxid = hexStringToBinary( hashStr );
   assert( binaryTxid.size() == sizeof(Sha256::Digest) );
   for( unsigned int i = 0; i < binaryTxid.size(); ++i )
   {
      prevHash[i] = binaryTxid[i];
   }

   // Load the outpoint index
   prevN = readInt( serialStream );

   // Load the signature script
   int scriptSize = readVarInt( serialStream );
   assert( scriptSize <= 10000 );
   string scriptSigStr( scriptSize * 2, 0 );
   serialStream.read( &scriptSigStr[0], scriptSigStr.size() );
   scriptSig = hexStringToBinary( scriptSigStr );

   // Load the sequence number
   sequence = readInt( serialStream );
}

void TxnInput::storeSerial( std::ostream& serialStream )
{
   serialStream << std::hex << std::setfill('0');
   for( auto elem : prevHash )
   {
      writeInt( serialStream, elem, sizeof(elem) );
   }
   writeInt( serialStream, prevN );
   writeVarInt( serialStream, scriptSig.size() );
   serialStream << scriptSig;
   writeInt( serialStream, sequence );
}

void TxnOutput::loadSerial( std::istream& serialStream )
{
   // Load the value
   value = readInt( serialStream, sizeof(value) );

   // Load the pubkey script
   int scriptSize = readVarInt( serialStream );
   string scriptPubKeyStr( scriptSize * 2, 0 );
   serialStream.read( &scriptPubKeyStr[0], scriptPubKeyStr.size() );
   scriptPubKey = hexStringToBinary( scriptPubKeyStr );
}

void TxnOutput::storeSerial( std::ostream& serialStream )
{
   writeInt( serialStream, value, sizeof(value) );
   writeVarInt( serialStream, scriptPubKey.size() );
   serialStream << scriptPubKey;
}

Transaction::Transaction( const std::string& serializedTxnStr )
{
   stringstream txnSerialStream( serializedTxnStr );

   version = readInt( txnSerialStream );

   // Load inputs
   inputs.resize( readVarInt(txnSerialStream) );
   for( auto& input : inputs )
      input.loadSerial( txnSerialStream );

   // Load outputs
   outputs.resize( readVarInt(txnSerialStream) );
   for( auto& output : outputs )
      output.loadSerial( txnSerialStream );

   lockTime = readInt( txnSerialStream );
}

Transaction::~Transaction()
{
}

void Transaction::storeSerial( std::ostream& serialStream )
{
   writeInt( serialStream, version );

   writeVarInt( serialStream, inputs.size() );
   for( auto& input : inputs )
      input.storeSerial( serialStream );

   writeVarInt( serialStream, outputs.size() );
   for( auto& output : outputs )
      output.storeSerial( serialStream );

   writeInt( serialStream, lockTime );
}
