#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "Sha256.h"
#include "Util.h"
#include "Script.h"

#include <string>
#include <limits>

class Transaction
{
public:
   struct Input
   {
      void deserialize( std::istream& inStream );
      void serialize( std::ostream& outStream ) const;

      Sha256::Digest prevHash;
      int            prevN;
      Script         scriptSig;
      int            sequence;
   };

   struct Output
   {
      void deserialize( std::istream& inStream );
      void serialize( std::ostream& outStream ) const;

      int64_t  value;
      Script   scriptPubKey;
   };

public:
   void serialize( std::ostream& outStream ) const;

public:
   int                  version;
   int                  lockTime;
   std::vector<Input>   inputs;
   std::vector<Output>  outputs;

public:
   static Transaction createCoinbase( int blockHeight,
                                      int64_t coinbaseValue,
                                      const ByteArray& pubKeyHash );

   static Transaction deserialize( const std::string& serializedTxnStr );
};

typedef Transaction::Input TxnInput;
typedef Transaction::Output TxnOutput;

#endif // !TRANSACTION_H
