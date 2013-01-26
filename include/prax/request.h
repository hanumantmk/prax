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
    QString sender;
    QString conn_id;
    QString path;
    QHash<QString, QString> headers;
    QUrl url;
    QHash<QString, QString> urlParams;
    QString body;
    bool disconnect;
    Request(QByteArray);

};

QDebug operator<<(QDebug, const Request&);

}

#endif
