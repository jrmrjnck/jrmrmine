/**
 * This is free and unencumbered software released into the public domain.
**/

#include "Script.h"

#include <cassert>

Script::Data::Data( int64_t value, int size )
{
   _data = nullptr;
   _intData = value;
   _size = size;
}

Script::Data::Data( const void* data, int size )
{
   _data = data;
   _size = size;
}

Script::Data::Data( const ByteArray& byteArray )
 : Data( byteArray.data(), byteArray.size() )
{
}

Script& Script::operator <<( const Data& data )
{
   assert( data._size <= 75 );
   push_back( data._size );

   auto src = reinterpret_cast<const uint8_t*>((data._data != nullptr)
                                               ? data._data
                                               : &data._intData
                                               );
   for( int i = 0; i < data._size; ++i )
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
