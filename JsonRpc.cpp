/*
 * Jonathan Doman
 * jonathan.doman@gmail.com
 */
#include "JsonRpc.h"

#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QEventLoop>

JsonRpc::JsonRpc( const QString& url, const QString& username, const QString& password )
 : _req(url)
{
   // Encode RPC username and password
   QByteArray authPair;
   authPair += username + ":" + password;
   QByteArray authHeader( "Basic " );
   authHeader += authPair.toBase64();

   // Set HTTP Headers
   _req.setRawHeader( "Authorization", authHeader );
   _req.setHeader( QNetworkRequest::ContentTypeHeader, "application/json" );
}

JsonRpc::~JsonRpc()
{
}

QJsonObject JsonRpc::call( const QString& method, const QJsonObject& data )
{
   // Create the JSON request
   QJsonObject obj = data;
   obj.insert( "method", method );
   QByteArray reqData = QJsonDocument( obj ).toJson( QJsonDocument::Compact );

   // Post request and wait for reply
   QNetworkAccessManager netManager;
   QNetworkReply* reply = netManager.post( _req, reqData );
   QEventLoop loop;
   QObject::connect( reply, SIGNAL(readyRead()), &loop, SLOT(quit()) );

   loop.exec();

   QJsonDocument replyDoc = QJsonDocument::fromJson( reply->readAll() );
   obj = replyDoc.object();
   return obj.value( "result" ).toObject();
}
