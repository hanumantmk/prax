#include "prax/countdown.h"

using namespace Prax;

Countdown::Countdown(QString in_addr, QString out_addr)
{
    page = new OffScreenWebPage;
    view = new QWebView;
    view->setPage(page);

    nzmqt::ZMQContext * context = nzmqt::createDefaultContext(this);
    context->start();

    in_socket = context->createSocket(nzmqt::ZMQSocket::TYP_PULL);
    in_socket->connectTo(in_addr);
    connect(in_socket, SIGNAL(messageReceived(const QList<QByteArray> &)), SLOT(gen_countdown(const QList<QByteArray> &)));

    out_socket = context->createSocket(nzmqt::ZMQSocket::TYP_PUB);
    out_socket->connectTo(out_addr);
}

void Countdown::gen_countdown(const QList<QByteArray> &request)
{
    qDebug() << "Got request" << request;
}
