#include "Script.h"

#include <cassert>

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
