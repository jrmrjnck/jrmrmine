/**
 * This is free and unencumbered software released into the public domain.
**/

#include "Transaction.h"

#include <sstream>
#include <cassert>
#include <iomanip>
#include <iostream>

using std::string;
using std::stringstream;

void TxnInput::deserialize( std::istream& serialStream )
{
   // Load the outpoint TXID
   string hashStr( sizeof(Sha256::Digest) * 2, 0 );
   serialStream.read( &hashStr[0], hashStr.size() );
   ByteArray binaryTxid = hexStringToBinary( hashStr );
   assert( binaryTxid.size() == sizeof(Sha256::Digest) );
   for( unsigned int i = 0; i < prevHash.size(); ++i )
   {
      prevHash[i] =   (binaryTxid[i * 4 + 0] << 0 * CHAR_BIT)
                    + (binaryTxid[i * 4 + 1] << 1 * CHAR_BIT)
                    + (binaryTxid[i * 4 + 2] << 2 * CHAR_BIT)
                    + (binaryTxid[i * 4 + 3] << 3 * CHAR_BIT);
   }

   // Load the outpoint index
   prevN = readInt<int>( serialStream );

   // Load the signature script
   int scriptSize = readVarInt( serialStream );
   assert( scriptSize <= 10000 );
   string scriptSigStr( scriptSize * 2, 0 );
   serialStream.read( &scriptSigStr[0], scriptSigStr.size() );
   scriptSig = Script::deserialize( scriptSigStr );

   // Load the sequence number
   sequence = readInt<int>( serialStream );
}

void TxnInput::serialize( std::ostream& serialStream ) const
{
   serialStream << std::hex << std::setfill('0');
   for( auto elem : prevHash )
   {
      writeInt( serialStream, elem );
   }
   writeInt( serialStream, prevN );
   writeVarInt( serialStream, scriptSig.size() );
   serialStream << scriptSig;
   writeInt( serialStream, sequence );
}

void TxnOutput::deserialize( std::istream& serialStream )
{
   // Load the value
   value = readInt<int64_t>( serialStream );

   // Load the pubkey script
   int scriptSize = readVarInt( serialStream );
   string scriptPubKeyStr( scriptSize * 2, 0 );
   serialStream.read( &scriptPubKeyStr[0], scriptPubKeyStr.size() );
   scriptPubKey = Script::deserialize( scriptPubKeyStr );
}

void TxnOutput::serialize( std::ostream& serialStream ) const
{
   writeInt( serialStream, value );
   writeVarInt( serialStream, scriptPubKey.size() );
   serialStream << scriptPubKey;
}

void Transaction::serialize( std::ostream& serialStream ) const
{
   writeInt( serialStream, version );

   writeVarInt( serialStream, inputs.size() );
   for( auto& input : inputs )
      input.serialize( serialStream );

   writeVarInt( serialStream, outputs.size() );
   for( auto& output : outputs )
      output.serialize( serialStream );

   writeInt( serialStream, lockTime );
}

ByteArray Transaction::id() const
{
   std::ostringstream stream;
   serialize( stream );
   ByteArray binaryData = hexStringToBinary( stream.str() );
   return Sha256::doubleHash( binaryData );
}

TransactionPtr Transaction::createCoinbase( int blockHeight,
                                            int64_t coinbaseValue,
                                            const ByteArray& pubKeyHash )
{
   TransactionPtr coinbaseTxn( new Transaction );

   coinbaseTxn->version = 1;
   coinbaseTxn->inputs.resize( 1 );
   auto& coinbaseInput = coinbaseTxn->inputs[0];
   coinbaseInput.prevHash.fill( 0 );
   coinbaseInput.prevN = -1;
   coinbaseInput.scriptSig << Script::Data(blockHeight)
                           << 0 << 0 << 0 << 0;
   coinbaseInput.sequence = 0;
   coinbaseTxn->outputs.resize( 1 );
   auto& coinbaseOutput = coinbaseTxn->outputs[0];
   coinbaseOutput.value = coinbaseValue;
   coinbaseOutput.scriptPubKey << OP_DUP << OP_HASH160
                               << Script::Data(pubKeyHash)
                               << OP_EQUALVERIFY << OP_CHECKSIG;
   coinbaseTxn->lockTime = 0;
   return coinbaseTxn;
}

TransactionPtr Transaction::deserialize( const std::string& serializedTxnStr )
{
   TransactionPtr txn( new Transaction );
   std::istringstream txnSerialStream( serializedTxnStr );

   txn->version = readInt<int>( txnSerialStream );

   // Load inputs
   txn->inputs.resize( readVarInt(txnSerialStream) );
   for( auto& input : txn->inputs )
      input.deserialize( txnSerialStream );

   // Load outputs
   txn->outputs.resize( readVarInt(txnSerialStream) );
   for( auto& output : txn->outputs )
      output.deserialize( txnSerialStream );

   txn->lockTime = readInt<int>( txnSerialStream );

   return txn;
}
