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
#include <thread>
#include <atomic>

using namespace std;

const char RPC_USERNAME[] = "admin1";
const char RPC_PASSWORD[] = "123";

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

bool searchNonce( const Sha256& headerHash, const std::string& target, uint32_t& nonce )
{
   uint8_t sum[32] = {};
   nonce = 0;
   auto start = chrono::system_clock::now();
   const unsigned int INTERVAL = 0x00010000;
   for( long int c = 0; c <= UINT_MAX; ++c, ++nonce )
   {
      // Periodically check whether the 1 sec limit has elapsed
      if( (c % INTERVAL) == 0 )
      {
         auto diff = chrono::system_clock::now() - start;
         auto ms = chrono::duration_cast<chrono::milliseconds>(diff).count();
         if( ms >= 1000 )
            return false;
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
      for( int i = 31; i >= 0; --i )
      {
         if( sum[i] < target[i] )
            return true;
      }
   }

   return false;
}

atomic_long hashCount;

bool work( JsonRpc& rpc )
{
   Json::Value req;
   auto resp = rpc.call( "getwork", req );

   if( !resp.isMember("data") || !resp.isMember("target") )
      return false;

   string header = resp["data"].asString();
   hexStringToBinary( header );
   swapEndianness( header );
   header.resize( 76 );

   string target = resp["target"].asString();
   hexStringToBinary( target );
   swapEndianness( target );

   Sha256 headerHash;
   headerHash.update( header.data(), CHAR_BIT*header.size() );

   uint32_t nonce = 0;
   bool success = searchNonce( headerHash, target, nonce );
   hashCount += nonce;
   if( !success )
      return true;

   // Set nonce value in return data
   string retData = resp["data"].asString();
   for( int i = 0; i < 4; ++i )
   {
      int byte = (nonce >> ((3-i)*8)) & 0xff;
      char buf[3];
      snprintf( buf, sizeof(buf), "%.2x", byte );
      retData[152+2*i] = buf[0];
      retData[153+2*i] = buf[1];
   }

   printf( "Found nonce %s\n", retData.c_str() );

   req["data"] = retData;
   resp = rpc.call( "getwork", req );

   return true;
}

void minerThread( int tid )
{
   printf( "Thread %d started\n", tid );
   JsonRpc rpc( "http://localhost:19001", RPC_USERNAME, RPC_PASSWORD );

   int failCount = 0;
   while( failCount < 10 )
   {
      if( tid == 0 )
      {
         static long int lastCount = 0;
         printf( "Hashes: %ld H/s\n", hashCount.load()-lastCount );
         lastCount = hashCount.load();
      }

      if( !work(rpc) )
         ++failCount;
   }

   printf( "Thread %d finished\n", tid );
}

int main( int /*argc*/, char** /*argv*/ )
{
   int numThreads = thread::hardware_concurrency();
   if( numThreads == 0 )
      numThreads = 4;
   thread threads[numThreads];

   for( int i = 0; i < numThreads; ++i )
   {
      threads[i] = thread( minerThread, i );
   }

   cout << numThreads << " mining thread(s) started" << endl;

   for( int i = 0; i < numThreads; ++i )
   {
      threads[i].join();
   }

   return 0;
}
