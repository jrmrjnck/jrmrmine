#include "Sha256.h"

#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTextStream>
#include <QObject>
#include <QtEndian>

#include <cassert>
#include <algorithm>
#include <cstdio>

const char RPC_USERNAME[] = "jrmrcoin";
const char RPC_PASSWORD[] = "ffb137";

static QTextStream outs( stdout );

// Given hex-encoded string, reverse bytes
void reverseHexBytes( QByteArray& ba )
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
void swapEndianness( QByteArray& ba )
{
   assert( ba.size() % 4 == 0 );

   for( int i = 0; i < ba.size(); i += 4 )
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
QByteArray bitsToTarget( uint32_t bits )
{
   QByteArray res( 32, 0 );
   uint8_t* data = reinterpret_cast<uint8_t*>(res.data());

   // Most significant 8 bits are the unsigned exponent, base 256
   int size = bits >> 24;

   // Lower 23 bits are mantissa
   int word = bits & 0x007fffff;

   if( size <= 3 )
   {
      word >>= 8*(3-size);
      data[31] = word & 0xFF; word >>= 8;
      data[30] = word & 0xFF; word >>= 8;
      data[29] = word & 0xFF; word >>= 8;
      data[28] = word & 0xFF;
   }
   else
   {
      int shf = size - 3;
      data[31-shf] = word & 0xFF; word >>= 8;
      data[30-shf] = word & 0xFF; word >>= 8;
      data[29-shf] = word & 0xFF; word >>= 8;
      data[28-shf] = word & 0xFF;
   }

   return res;
}


int main( int argc, char* argv[] )
{
   QCoreApplication cpuMiner( argc, argv );

   QNetworkAccessManager netManager;

   QNetworkRequest req( QUrl("http://127.0.0.1:8332") );

   // Encode RPC username and password
   QByteArray authPair( RPC_USERNAME );
   authPair.append( ":" );
   authPair.append( RPC_PASSWORD );
   QByteArray authHeader( "Basic " );
   authHeader.append( authPair.toBase64() );

   // Set HTTP Headers
   req.setRawHeader( "Authorization", authHeader );
   req.setHeader( QNetworkRequest::ContentTypeHeader, "application/json" );

   // Create the JSON request
   QJsonObject reqObj;
   reqObj.insert( "method", QString("getwork") );
   reqObj.insert( "params", QJsonArray() );
   QByteArray reqData = QJsonDocument(reqObj).toJson(QJsonDocument::Compact);

   // Post request and wait for reply
   QNetworkReply* reply = netManager.post( req, reqData );
   QEventLoop loop;
   QObject::connect( reply, SIGNAL(readyRead()), &loop, SLOT(quit()) );

   loop.exec();

   QJsonDocument replyDoc = QJsonDocument::fromJson( reply->readAll() );
   QJsonObject resObj = replyDoc.object().value( "result" ).toObject();

   // Big-endian for easy reading
   //QByteArray version = "00000001";
   //QByteArray hashPrevBlock = "00000000000008a3a41b85b8b29ad444def299fee21793cd8b9e567eab02cd81";
   //QByteArray hashMerkleRoot = "2b12fcf1b09288fcaff797d71e950e71ae42b91e8bdb2304758dfcffc2b620e3";
   //QByteArray time = "4dd7f5c7";
   //QByteArray bits = "1a44b9f2";
   //QByteArray target = bitsToTarget( bits.toUInt(NULL,16) );

   //reverseHexBytes( version );
   //reverseHexBytes( hashPrevBlock );
   //reverseHexBytes( hashMerkleRoot );
   //reverseHexBytes( time );
   //reverseHexBytes( bits );
   //std::reverse( target.begin(), target.end() );

   //QByteArray header = version + hashPrevBlock + hashMerkleRoot + time + bits;
   QByteArray header = resObj.value("data").toString().toLocal8Bit();
   header = QByteArray::fromHex( header );
   swapEndianness( header );
   header.truncate( 76 );

   QByteArray target = resObj.value("target").toString().toLocal8Bit();
   target = QByteArray::fromHex( target );
   swapEndianness( target );

   qDebug() << header.toHex();
   qDebug() << target.toHex();

   Sha256 headerHash;
   headerHash.update( header.data(), CHAR_BIT*header.size() );

   uint8_t sum[32] = {};
   uint32_t nonce;
   QTime time;
   time.start();
   for( nonce = 0; ; ++nonce )
   {
      if( nonce % 0x00100000 == 0 )
      {
         outs << "\r" << static_cast<double>(0x100000)/time.restart() << " kH/s" << flush;
      }

      uint32_t leNonce = qToLittleEndian( nonce );

      Sha256 hash1( headerHash );
      hash1.update( &leNonce, CHAR_BIT*sizeof(leNonce) );
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

   qDebug() << "Found nonce" << hex << nonce;
   printSum( sum );

   return 0;
}
