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

const int ALPHABET_INVALID_LETTER = -1;

// The "system radix" corresponds to the range of the smallest addressable unit,
// aka char in C. Data converted to system radix will therefore look like its
// raw big-endian representation.
const int SYSTEM_RADIX = 1 << CHAR_BIT;

const Radix::Alphabet BASE_58_ALPHABET(
   "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz"
   );

Radix::Alphabet::Alphabet( const std::string& encodeTable )
 : _encodeTable( encodeTable )
{
   // Need to find bounds manually since characters in the alphabet may be
   // out-of-order relative to their numeric value.
   _lowerBound = std::numeric_limits<char>::max();
   _upperBound = 0;

   for( char ch : encodeTable )
   {
      if( ch < _lowerBound )
      {
         _lowerBound = ch;
      }

      if( ch > _upperBound )
      {
         _upperBound = ch;
      }
   }

   // The decode table will be larger than the encode table if there are gaps
   // in the alphabet.
   int alphabetRange = _upperBound - _lowerBound;
   _decodeTable.resize( alphabetRange, ALPHABET_INVALID_LETTER );

   for( unsigned int i = 0; i < _encodeTable.size(); ++i )
   {
      char ch = _encodeTable[i];
      _decodeTable[ch - _lowerBound] = i;
   }
}

ByteArray Radix::base58DecodeCheck( const std::string& base58Str )
{
   const Alphabet& srcAlphabet = BASE_58_ALPHABET;

   ByteArray input = decodeAlphabet( base58Str, srcAlphabet );
   ByteArray output = convert( input, srcAlphabet._encodeTable.size(), SYSTEM_RADIX );

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

   for( int ch : inputStr )
   {
      ch -= alphabet._lowerBound;
      assert( ch >= 0 && ch <= alphabet._upperBound );

      int value = alphabet._decodeTable[ch];
      assert( value != ALPHABET_INVALID_LETTER );

      output.push_back( value );
   }

   return output;
}

ByteArray Radix::convert( const ByteArray& input, int srcRadix, int destRadix )
{
   assert( srcRadix <= SYSTEM_RADIX );
   assert( destRadix <= SYSTEM_RADIX );

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
