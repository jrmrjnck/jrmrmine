#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <istream>
#include <cstdint>
#include <vector>

#define SATOSHIS_PER_BITCOIN 100000000

#define ARRAY_SIZE(array) static_cast<int>(sizeof(array) / sizeof(*array))

typedef std::vector<uint8_t>  ByteArray;

void reverseHexBytes( std::string& ba );
int64_t readInt( std::istream& ss, size_t size = 4 );
int64_t readVarInt( std::istream& ss );
void writeInt( std::ostream& ss, int64_t n, size_t size = 4 );
void writeVarInt( std::ostream& ss, int64_t n );
void printSum( const uint8_t* sum );
std::string bitsToTarget( uint32_t bits );
int hexToInt( char c );
ByteArray hexStringToBinary( const std::string& str );
std::ostream& operator <<( std::ostream& outputStream, const ByteArray& byteArray );
int isBigEndian();

#endif // UTIL_H
