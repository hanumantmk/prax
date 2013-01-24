#include "prax/request.h"
#include <QHash>
#include <QString>
#include <QUrl>

using namespace Prax;

Request::Request(QByteArray ba)
{
    QByteArray left, ign;

    QByteArray space(" ");

    QList<QByteArray> results = utils::split(ba, space, 3);

    sender = QString(results[0]);
    conn_id = QString(results[1]);
    path = QString(results[2]);

    QByteArray unparsed_headers = utils::parse_netstring(results[3], left);

    headers = utils::parse_json(QString(unparsed_headers));

    body = QString(utils::parse_netstring(left, ign));

    //check disconnect flag
    disconnect = false;

    QByteArray url_bytes(headers.value("URI").toUtf8());
    url = QUrl::fromEncoded(url_bytes);

    if (headers["METHOD"] == "JSON" && body == "{\"type\":\"disconnect\"}") disconnect = true;
}

QDebug Prax::operator<<(QDebug dbg, const Request& req)
{
    dbg << "{\n" <<
        "sender : " << req.sender << ",\n" <<
        "conn_id : " << req.conn_id << ",\n" <<
        "body : " << req.body << ",\n" <<
        "path : " << req.path << ",\n" <<
        "disconnect : " << req.disconnect << ",\n" <<
        "url : " << req.url << ",\n" <<
        "headers : " << req.headers <<
    "}\n";

    return dbg;
}
