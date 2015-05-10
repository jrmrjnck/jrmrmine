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
      Data( int64_t value, int size )
      {
         _data = nullptr;
         _intData = value;
         _size = size;
      }

      Data( const void* data, int size )
      {
         _data = data;
         _size = size;
      }

   private:
      int64_t        _intData;
      const void*    _data;
      int            _size;
   };

public:
   Script() {}
   ~Script() {}

   void operator =( const ByteArray& byteArray )
   {
      static_cast<ByteArray>(*this) = byteArray;
   }

   Script& operator <<( const Data& data );
   Script& operator <<( uint8_t byte );
};

#endif // !SCRIPT_H
