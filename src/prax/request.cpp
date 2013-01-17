#include "prax/request.h"
#include <QHash>
#include <QString>
#include <QUrl>

using namespace Prax;

Request::Request(QByteArray ba)
{
    std::string result(static_cast<const char *>(ba.data()), ba.size());

    std::vector<std::string> results = utils::split(result, " ", 3);

    sender = results[0];
    conn_id = results[1];
    path = results[2];

    std::string unparsed_headers = utils::parse_netstring(results[3], body);

    headers = utils::parse_json(unparsed_headers);

    std::string ign;

    body = utils::parse_netstring(body, ign);

    //check disconnect flag
    disconnect = false;
    QHash<QString, QString>::const_iterator it = headers.constBegin();
    while (it != headers.constEnd()) {
	// qDebug() << "header: " << it.key() << " : " << it.value();
        if (QString::compare(it.key(), QString("METHOD"), Qt::CaseSensitive) &&
            QString::compare(it.value(), QString("JSON"), Qt::CaseSensitive) &&
                        body == "{\"type\":\"disconnect\"}") {
                disconnect = true;
                break;
        }
	it++;
    }

    QByteArray url_bytes(headers.value("URI").toUtf8());
    url = QUrl::fromEncoded(url_bytes);
}

QDebug Prax::operator<<(QDebug dbg, const Request& req)
{
    dbg << "{ " <<
        "sender : " << req.sender.c_str() << ",\n" <<
        "conn_id : " << req.conn_id.c_str() << ",\n" <<
        "body : " << req.body.c_str() << ",\n" <<
        "path : " << req.path.c_str() << ",\n" <<
        "disconnect : " << req.disconnect << ",\n" <<
        "headers: {\n";

    QHash<QString, QString>::const_iterator it = req.headers.constBegin();
    while (it != req.headers.constEnd()) {
        dbg << "  " << it.key() << " : " << it.value() << ",\n";
	it++;
    }
    dbg << "}\n}";

    return dbg;
}
