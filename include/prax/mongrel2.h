#ifndef PRAX_MONGREL2_H
#define PRAX_MONGREL2_H

#include <QtCore>
#include <QTimer>

#include "prax/request.h"
#include "nzmqt/nzmqt.hpp"

namespace Prax {

class Mongrel2 : public QObject {
    Q_OBJECT

    nzmqt::ZMQSocket * in_socket;
    nzmqt::ZMQSocket * out_socket;

public:
    Mongrel2(QString inAddr, QString outAddr);

private slots:
    void zmqRequest(const QList<QByteArray> &req);

signals:
    void request(Request * req);

public slots:
    void reply(Request * req, QByteArray bytes);
    void endConnection(Request * req);

};

}

#endif
