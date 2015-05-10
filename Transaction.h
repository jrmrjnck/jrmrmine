#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "Sha256.h"
#include "Util.h"
#include "Script.h"

#include <string>
#include <limits>

struct Transaction
{
   struct Input
   {
      void loadSerial( std::istream& serialStream );
      void storeSerial( std::ostream& serialStream );

      Sha256::Digest prevHash;
      int            prevN;
      Script         scriptSig;
      int            sequence;
   };

   struct Output
   {
      void loadSerial( std::istream& serialStream );
      void storeSerial( std::ostream& serialStream );

      int64_t  value;
      Script   scriptPubKey;
   };

   Transaction() {}
   Transaction( const std::string& serializedTxnStr );
   ~Transaction();

   void storeSerial( std::ostream& serialStream );

   int                  version;
   int                  lockTime;
   std::vector<Input>   inputs;
   std::vector<Output>  outputs;
};

typedef Transaction::Input TxnInput;
typedef Transaction::Output TxnOutput;

#endif // !TRANSACTION_H
