/*
 * Jonathan Doman
 * jonathan.doman@gmail.com
 */

#include "Sha256.h"
#include "JsonRpc.h"

#include <cassert>
#include <algorithm>
#include <cstdio>
#include <iostream>
#include <string>
#include <ctype.h>
#include <climits>
#include <chrono>

using namespace std;

const char RPC_USERNAME[] = "jrmrcoin";
const char RPC_PASSWORD[] = "ffb137";

// Given hex-encoded string, reverse bytes
void reverseHexBytes( string& ba )
{
   assert( ba.size() % 2 == 0 );

   int i = 0;
   int j = ba.size() - 2;
   while( i < j )
   {
      char t1 = ba[i];
      char t2 = ba[i+1];
      ba[i]   = ba[j];
      ba[i+1] = ba[j+1];
      ba[j]   = t1;
      ba[j+1] = t2;
      i += 2;
      j -= 2;
   }
}

// Reverse byte order within 4-byte words
// Input must be binary encoded (not hex text)
void swapEndianness( string& ba )
{
   assert( ba.size() % 4 == 0 );

   for( unsigned i = 0; i < ba.size(); i += 4 )
   {
      char t0 = ba[i+0];
      char t1 = ba[i+1];
      ba[i+0] = ba[i+3];
      ba[i+1] = ba[i+2];
      ba[i+2] = t1;
      ba[i+3] = t0;
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

void hexStringToBinary( string& str )
{
   assert( str.size() % 2 == 0 );

   int o = 0;
   for( unsigned i = 0; i < str.size(); i += 2 )
   {
      uint8_t val = hexToInt( str[i] );
      val <<= 4;
      val |= hexToInt( str[i+1] );
      str[o++] = val;
   }

   str.resize( o );
}

int main( int /*argc*/, char** /*argv*/ )
{
   JsonRpc rpc( "http://localhost:8332", RPC_USERNAME, RPC_PASSWORD );
   Json::Value data;
   data["params"] = Json::arrayValue;
   data = rpc.call( "getwork", data );

   string header = data["data"].asString();
   cout << "data: " << header << endl;
   hexStringToBinary( header );
   swapEndianness( header );
   header.resize( 76 );

   string target = data["target"].asString();
   cout << "target: " << target << endl;
   hexStringToBinary( target );
   swapEndianness( target );

   Sha256 headerHash;
   headerHash.update( header.data(), CHAR_BIT*header.size() );

   uint8_t sum[32] = {};
   uint32_t nonce;
   auto start = chrono::high_resolution_clock::now();
   const unsigned int INTERVAL = 0x00100000;
   for( nonce = 0; ; ++nonce )
   {
      if( nonce % INTERVAL == 0 )
      {
         auto diff = chrono::high_resolution_clock::now() - start;
         auto ms = chrono::duration_cast<chrono::milliseconds>(diff).count();
         cout << '\r' << static_cast<double>(INTERVAL)/ms << " kH/s   " << flush;
         start = chrono::high_resolution_clock::now();
      }

      Sha256 hash1( headerHash );
      hash1.update( &nonce, CHAR_BIT*sizeof(nonce) );
      hash1.digest( sum );

      Sha256 hash2;
      hash2.update( sum, CHAR_BIT*sizeof(sum) );
      hash2.digest( sum );

      // Quick test for good hash
      if( reinterpret_cast<uint32_t*>(sum)[7] != 0 )
         continue;

      // Full comparison for meeting the target
      bool success = false;
      for( int i = 31; i >= 0; --i )
      {
         if( sum[i] < target[i] )
         {
            success = true;
            break;
         }
      }
      if( success )
         break;
   }

   cout << "\nFound nonce" << hex << nonce;
   printSum( sum );

   return 0;
}
