/**
 * This is free and unencumbered software released into the public domain.
**/
#ifndef RADIX_H
#define RADIX_H

#include "Util.h"

#include <array>

/*
 * Set of functions for converting data between different radixes, and
 * converting between "pretty" representations of a specific radix.
 *
 * Data is represented as ByteArrays, so the highest radix this class supports
 * is the range of a byte (e.g. 256). Data in this radix (system radix) is
 * considered "raw".
 */
class Radix
{
public:
   /*
    * An Alphabet is used to map data in a specific radix to a set of
    * characters, for use in printing, transmission, etc.
    */
   struct Alphabet
   {
      /*
       * Precompute a decode table from the string of encode characters, in
       * order to speed up conversions.
       */
      Alphabet( const std::string& encodeTable );

   private:
      std::string       _encodeTable;
      std::vector<int>  _decodeTable;
      char              _lowerBound;
      char              _upperBound;

      friend class Radix;
   };

public:
   /*
    * Decode a string from the bitcoin base 58 alphabet to raw data (system
    * radix) and verify the checksum.
    */
   static ByteArray base58DecodeCheck( const std::string& base58Str );

   /*
    * Decode a string from an arbitrary alphabet to data in the same radix (no
    * radix conversion is performed).
    */
   static ByteArray decodeAlphabet( const std::string& inputStr, const Alphabet& alphabet );

   /*
    * Convert data from one radix to another.
    */
   static ByteArray convert( const ByteArray& input, int srcRadix, int destRadix );
};

/*
 * The bitcoin base 58 alphabet
 */
extern const Radix::Alphabet BASE_58_ALPHABET;

#endif // !RADIX_H
