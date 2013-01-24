#include "prax/mongrel2.h"

using namespace Prax;

Mongrel2::Mongrel2(QString inAddr, QString outAddr)
{
    nzmqt::ZMQContext * context = nzmqt::createDefaultContext(this);
    context->start();

    in_socket = context->createSocket(nzmqt::ZMQSocket::TYP_PULL);
    in_socket->connectTo(inAddr);
    connect(in_socket, SIGNAL(messageReceived(const QList<QByteArray> &)), SLOT(zmqRequest(const QList<QByteArray> &)));

    out_socket = context->createSocket(nzmqt::ZMQSocket::TYP_PUB);
    out_socket->connectTo(outAddr);
}

void Mongrel2::zmqRequest(const QList<QByteArray> &req)
{
    Request * r = new Request(req[0]);

    qDebug() << "Got request " << *r;

    emit request(r);
}

void Mongrel2::reply(Request * req, QByteArray bytes)
{
    QStringList idents;
    idents.push_back(req->conn_id);

    qDebug() << "delivering reply for " << req->sender << idents;

    utils::deliver(req->sender, idents, bytes, out_socket);
}

void Mongrel2::endConnection(Request * req)
{
    QByteArray bytes("");

    reply(req, bytes);
}
