#include "Util.h"

#include <cassert>
#include <climits>
#include <iomanip>

using namespace std;

// Given hex-encoded string, reverse bytes
void reverseHexBytes( string& ba )
{
   assert( ba.size() % 2 == 0 );

   int i = 0;
   int j = ba.size() - 2;
   while( i < j )
   {
      char t1 = ba[i];
      char t2 = ba[i + 1];

      ba[i]     = ba[j];
      ba[i + 1] = ba[j + 1];
      ba[j]     = t1;
      ba[j + 1] = t2;

      i += 2;
      j -= 2;
   }
}

int64_t readInt( std::istream& ss, size_t size )
{
   assert( size <= sizeof(int64_t) );

   // TODO: figure out better way to use stringstream
   string str( size * 2, '\0' );
   for( auto& c : str )
      ss.get( c );
   reverseHexBytes( str );
   return stoi( str, nullptr, 16 );
}

int64_t readVarInt( std::istream& ss )
{
   uint8_t prefix = hexToInt(ss.get()) << 4 | hexToInt(ss.get());
   switch( prefix )
   {
   case 0xff:
      return readInt( ss, 8 );
   case 0xfe:
      return readInt( ss, 4 );
   case 0xfd:
      return readInt( ss, 2 );
   default:
      return prefix;
   }
}

void writeInt( std::ostream& ss, int64_t n, size_t size )
{
   ss << hex << setfill('0');
   for( unsigned i = 0; i < size; ++i )
   {
      unsigned int byte = n & 0xff;
      ss << setw(2) << byte;
      n >>= CHAR_BIT;
   }
}

void writeVarInt( std::ostream& ss, int64_t n )
{
   uint64_t un = n;
   ss << hex << setfill('0');
   if( un < 0xfd )
   {
      ss << setw(2) << static_cast<unsigned int>(un);
   }
   else if( un < 0xffff )
   {
      ss << "fd";
      ss << setw(4) << static_cast<unsigned int>(un);
   }
   else if( un < 0xffffffff )
   {
      ss << "fe";
      ss << setw(8) << static_cast<unsigned int>(un);
   }
   else
   {
      ss << "ff";
      ss << setw(16) << un;
   }
}

void printSum( const uint8_t* sum )
{
   for( int i = 0; i < 32; ++i )
   {
      printf( "%.2x", sum[i] );
   }
   printf( "\n" );
}

// See bitcoin/bitnum.h/CBigNum::SetCompact
// Returns big-endian string
string bitsToTarget( uint32_t bits )
{
   string res( 32, 0 );

   // Most significant 8 bits are the unsigned exponent, base 256
   int size = bits >> 24;

   // Lower 23 bits are mantissa
   int word = bits & 0x007fffff;

   if( size <= 3 )
   {
      word >>= 8*(3-size);
      res[31] = word & 0xFF; word >>= 8;
      res[30] = word & 0xFF; word >>= 8;
      res[29] = word & 0xFF; word >>= 8;
      res[28] = word & 0xFF;
   }
   else
   {
      int shf = size - 3;
      res[31-shf] = word & 0xFF; word >>= 8;
      res[30-shf] = word & 0xFF; word >>= 8;
      res[29-shf] = word & 0xFF; word >>= 8;
      res[28-shf] = word & 0xFF;
   }

   return res;
}

int hexToInt( char c )
{
   c = tolower( c );
   if( c >= '0' && c <= '9' )
      return c - '0';
   else if( c >= 'a' && c <= 'f' )
      return c - 'a' + 10;
   else
      return -1;
}

ByteArray hexStringToBinary( const string& str )
{
   assert( str.size() % 2 == 0 );

   ByteArray result;

   for( unsigned i = 0; i < str.size(); i += 2 )
   {
      uint8_t val = hexToInt( str[i] );
      val <<= 4;
      val |= hexToInt( str[i+1] );
      result.push_back( val );
   }

   return result;
}

std::ostream& operator <<( std::ostream& outputStream, const ByteArray& byteArray )
{
   outputStream << std::hex << std::setfill('0');

   for( auto byte : byteArray )
   {
      outputStream << std::setw(2) << static_cast<int>(byte);
   }

   return outputStream;
}

int isBigEndian()
{
   union
   {
      uint32_t i;
      char c[4];
   } bint = {0x01020304};

   return bint.c[0] == 1;
}
