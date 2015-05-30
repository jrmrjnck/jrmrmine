/**
 * This is free and unencumbered software released into the public domain.
**/

#include "Radix.h"
#include "Sha256.h"

#include <cassert>
#include <algorithm>
#include <cmath>
#include <climits>
#include <iostream>
#include <iomanip>

const Radix::Alphabet BASE_58_ALPHABET = {
   "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz",
   {
   /* 1-9 */ 0, 1, 2, 3, 4, 5, 6, 7, 8,
   /* :-@ */ -1, -1, -1, -1, -1, -1, -1,
   /* A-Z */ 9, 10, 11, 12, 13, 14, 15, 16, -1, 17, 18, 19, 20, 21, -1, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32,
   /* [-` */ -1, -1, -1, -1, -1, -1,
   /* a-z */ 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, -1, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57
   }
};

ByteArray Radix::base58DecodeCheck( const std::string& base58Str )
{
   ByteArray input = decodeAlphabet( base58Str, BASE_58_ALPHABET );

   ByteArray output = convert( input, 58, 256 );

   // If we have enough data to constitute a payload, do a
   // SHA256(SHA256(payload)) check
   if( output.size() > 4 )
   {
      ByteArray checkCode( output.end() - 4, output.end() );
      output.erase( output.end() - 4, output.end() );

      auto rawDigest = Sha256::doubleHash( output );

      if( !std::equal(checkCode.begin(), checkCode.end(), rawDigest.begin()) )
         throw std::runtime_error( "address check code failed" );
   }

   return output;
}

ByteArray Radix::decodeAlphabet( const std::string& inputStr, const Alphabet& alphabet )
{
   ByteArray output;
   output.reserve( inputStr.size() );

   for( auto ch : inputStr )
   {
      ch -= alphabet.encodeTable[0];
      assert( ch >= 0 && ch <= alphabet.encodeTable.back() );

      int value = alphabet.decodeTable[ch];
      assert( value != -1 );

      output.push_back( value );
   }

   return output;
}

ByteArray Radix::convert( const ByteArray& input, int srcRadix, int destRadix )
{
   assert( srcRadix <= 256 );
   assert( destRadix <= 256 );

   auto it = std::find_if( input.begin(), input.end(), [](int8_t a){return a != 0;} );
   int inputSize = input.size() - (it - input.begin());

   // Equivalent of (inputSize / log_srcRadix(destRadix))
   int outputSize = std::ceil(inputSize * std::log(srcRadix) / std::log(destRadix)) + 0.5;

   ByteArray output( outputSize, 0 );

   // The conversion algorithm takes a src digit (from the top) and attempts to
   // stuff it into the dest number (at the bottom). As the digits in the dest
   // overflow, the remainder is cascaded into the next upper digit.
   for( auto srcUnit : input )
   {
      int value = srcUnit;
      for( int outputPos = outputSize - 1; outputPos >= 0; --outputPos )
      {
         value += srcRadix * output[outputPos];
         output[outputPos] = value % destRadix;
         value /= destRadix;
      }
   }

   // The output size calculation has to be conservative, so there may be
   // leading bytes in the output which have to be removed. Shouldn't be more
   // than one, though.
   int removedCount = 0;
   while( output[0] == 0 )
   {
      output.erase( output.begin() );
      ++removedCount;
   }
   assert( removedCount <= 1 );

   return output;
}
