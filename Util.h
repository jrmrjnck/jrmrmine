#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <istream>
#include <cstdint>

void reverseHexBytes( std::string& ba );
int64_t readInt( std::istream& ss, int size = 4 );
int64_t readVarInt( std::istream& ss );
void writeInt( std::ostream& ss, int64_t n, int size = 4 );
void writeVarInt( std::ostream& ss, int64_t n );
void swapEndianness( std::string& ba );
void printSum( const uint8_t* sum );
std::string bitsToTarget( uint32_t bits );
int hexToInt( char c );
void hexStringToBinary( std::string& str );

#endif // UTIL_H
