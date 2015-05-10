#ifndef RADIX_H
#define RADIX_H

#include "Util.h"

#include <array>

class Radix
{
public:
   struct Alphabet
   {
      const std::string       encodeTable;
      const std::vector<int>  decodeTable;
   };

public:
   static ByteArray base58DecodeCheck( const std::string& base58Str );

   static ByteArray decodeAlphabet( const std::string& inputStr, const Alphabet& alphabet );

   static ByteArray convert( const ByteArray& input, int srcRadix, int destRadix );
};

#endif // !RADIX_H
