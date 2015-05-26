#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <istream>
#include <cstdint>
#include <vector>
#include <iomanip>
#include <climits>

#define SATOSHIS_PER_BITCOIN 100000000

#define ARRAY_SIZE(array) static_cast<int>(sizeof(array) / sizeof(*array))

typedef std::vector<uint8_t>  ByteArray;

void reverseHexBytes( std::string& ba );

std::string bitsToTarget( uint32_t bits );
int hexToInt( char c );
ByteArray hexStringToBinary( const std::string& str );
std::ostream& operator <<( std::ostream& outputStream, const ByteArray& byteArray );
bool isLittleEndian();
bool isPowerOfTwo( int n );

//
// Functions that can read and write integers on a stream of hex characters
//
int64_t  readVarInt( std::istream& ss );
void     writeVarInt( std::ostream& ss, int64_t n );

template<typename T>
T readInt( std::istream& ss )
{
   std::string str( sizeof(T) * 2, '\0' );
   for( auto& ch : str )
      ss.get( ch );

   reverseHexBytes( str );

   return static_cast<T>(std::stoll(str,nullptr,16));
}

template<typename T>
void writeInt( std::ostream& ss, T n )
{
   ss << std::hex << std::setfill('0');
   for( unsigned int i = 0; i < sizeof(T); ++i )
   {
      ss << std::setw(2) << (n & 0xff);
      n >>= CHAR_BIT;
   }
}

#endif // UTIL_H
