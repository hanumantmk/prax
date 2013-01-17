#ifndef PRAX_REQUEST_H
#define PRAX_REQUEST_H

#include <QByteArray>
#include <QDebug>
#include <QHash>
#include <QString>
#include <QUrl>

#include "prax/utils.h"

namespace Prax {

class Request {
    public:
        std::string sender;
        std::string conn_id;
        std::string path;
        QHash<QString, QString> headers;
	QUrl url;
        std::string body;
        bool disconnect;
        Request(QByteArray);
};

QDebug operator<<(QDebug, const Request&);

}

#endif
