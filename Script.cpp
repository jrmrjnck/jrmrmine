/**
 * This is free and unencumbered software released into the public domain.
**/

#include "Script.h"

#include <cassert>

Script::Data::Data( int64_t value )
{
   _data = nullptr;
   _intData = value;
   _size = 0;

   // See how many bytes are required to hold the value
   // i.e. discard the top zero bytes
   while( (value & ((1 << (++_size * 8)) - 1)) != value );

   assert( _size <= sizeof(value) );
}

Script::Data::Data( const ByteArray& byteArray )
{
   _data = byteArray.data();
   _size = byteArray.size();
}

Script& Script::operator <<( const Data& data )
{
   assert( data._size <= 75 );
   push_back( data._size );

   assert( isLittleEndian() );
   auto src = reinterpret_cast<const uint8_t*>((data._data != nullptr)
                                               ? data._data
                                               : &data._intData
                                               );
   for( unsigned i = 0; i < data._size; ++i )
   {
      push_back( src[i] );
   }

   return *this;
}

Script& Script::operator <<( uint8_t byte )
{
   push_back( byte );
   return *this;
}

Script Script::deserialize( const std::string& serialized )
{
   Script script;
   static_cast<ByteArray&>(script) = hexStringToBinary( serialized );
   return script;
}
