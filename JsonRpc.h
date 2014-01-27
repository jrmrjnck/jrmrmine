/*
 * Jonathan Doman
 * jonathan.doman@gmail.com
 */
#ifndef JSONRPC_H
#define JSONRPC_H

#include <QNetworkRequest>
#include <QJsonObject>

class JsonRpc
{
public:
   JsonRpc( const QString& url, const QString& username, const QString& password );
   ~JsonRpc();

   QJsonObject call( const QString& method, const QJsonObject& data );

private:
   QNetworkRequest _req;
};

#endif // !JSONRPC_H
