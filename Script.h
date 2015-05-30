/**
 * This is free and unencumbered software released into the public domain.
**/
#ifndef SCRIPT_H
#define SCRIPT_H

#include "Util.h"

enum OpCode
{
   OP_DUP            = 118,
   OP_EQUALVERIFY    = 136,
   OP_HASH160        = 169,
   OP_CHECKSIG       = 172
};

class Script : public ByteArray
{
public:
   class Data
   {
      friend class Script;

   public:
      Data( int64_t value, int size );
      Data( const void* data, int size );
      Data( const ByteArray& byteArray );

   private:
      int64_t        _intData;
      const void*    _data;
      int            _size;
   };

public:
   Script& operator <<( const Data& data );
   Script& operator <<( uint8_t byte );

   static Script deserialize( const std::string& serialized );
};

#endif // !SCRIPT_H
