#include <gd.h>

#include "prax/request.h"
#include "prax/webwatch.h"

using namespace Prax;

WebWatch::WebWatch(QString in_addr, QString out_addr)
{
    page = new OffScreenWebPage;
    view = new QWebView;
    view->setPage(page);

    request = NULL;
    pgdimage = NULL;

    connect(page->networkAccessManager(), SIGNAL(finished(QNetworkReply*)), SLOT(gotReply(QNetworkReply*)));

    connect(page, SIGNAL(loadFinished(bool)), SLOT(capturePage()));

    connect(page->networkAccessManager(), SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), SLOT(sslErrors(QNetworkReply*,QList<QSslError>)));

    page->setViewportSize(pageSize);

    page->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
    page->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);

    nzmqt::ZMQContext * context = nzmqt::createDefaultContext(this);
    context->start();

    in_socket = context->createSocket(nzmqt::ZMQSocket::TYP_PULL);
    in_socket->connectTo(in_addr);
    connect(in_socket, SIGNAL(messageReceived(const QList<QByteArray> &)), SLOT(gen_page(const QList<QByteArray> &)));

    out_socket = context->createSocket(nzmqt::ZMQSocket::TYP_PUB);
    out_socket->connectTo(out_addr);
}

void WebWatch::gen_page(const QList<QByteArray> &msg)
{
    if (request) delete request;
    if (pgdimage) {
        gdImageDestroy(pgdimage);
        pgdimage = NULL;
    }

    qDebug() << "Got request";
    Request * req = new Request(msg[0]);
    qDebug() << *req;

    new_request = true;

    request = req;

    page->mainFrame()->load(QString("http://127.0.0.1:6767/static/foo.html"));
}

void WebWatch::gen_next_page()
{
    qDebug() << "generating a new page...";
    page->mainFrame()->load(QString("http://127.0.0.1:6767/static/foo.html"));
}

void WebWatch::capturePage()
{
    qDebug() << "Status: " << statusCode;

    if( statusCode != 0 &&
        statusCode != 301 &&
        statusCode != 302 &&
        statusCode != 303
       ) {

        QSize contentSize = page->mainFrame()->contentsSize();

        qDebug() << "view sizeHint: " << view->sizeHint();
        qDebug() << "page viewportSize: " << page->viewportSize();
        qDebug() << "page mainFrame contentsSize: " << page->mainFrame()->contentsSize();
        qDebug() << "page mainFrame documentElement geometry: " << page->mainFrame()->documentElement().geometry();
        qDebug() << "contentSize: " << contentSize;

        //page->setViewportSize(contentSize);

        //view->resize(contentSize); // for some reason this fixes garbage that would show up on the right ( past the 800 pixel mark )

        view->repaint();

        gdImage * gdimage = NULL;

        QRect grabRect;

        grabRect = QRect(0, 0, pageSize.width(), pageSize.height());

        int out_size;
        void * out_buf;

        QImage qimg = QPixmap::grabWidget(view, grabRect).toImage().convertToFormat(QImage::Format_Indexed8,0);

        QByteArray ba;
        QBuffer buffer(&ba);
        buffer.open(QIODevice::WriteOnly);
        qimg.save(&buffer, "PNG");
        buffer.close();

        gdimage = gdImageCreateFromPngPtr(ba.length(), ba.data());

        if (new_request) {
            new_request = false;
            out_buf = gdImageGifAnimBeginPtr(gdimage, &out_size, 1, -1);

            sendData(out_buf, out_size);

            gdFree(out_buf);
        }

        out_buf = gdImageGifAnimAddPtr(gdimage, &out_size, 1, 0, 0, 100, gdDisposalNone, pgdimage);
        sendData(out_buf, out_size);
        gdFree(out_buf);

        if ( pgdimage ) gdImageDestroy(pgdimage);
        pgdimage = gdimage;

        //out_buf = gdImageGifAnimEndPtr(&out_size);
        //sendData(out_buf, out_size);
        //gdFree(out_buf);
        //gdImageDestroy(gdimage);
    }
    else if(statusCode != 0) {
        statusCode = 0;
        qDebug() << "Redirecting to: " + redirectUrl.toString();
        if(page->mainFrame()->url().toString().isEmpty()) {
            qDebug() << "about:blank";
            page->mainFrame()->load(this->redirectUrl);
            qDebug() << "Loading";
        }
    }

    // This should ensure that the program never hangs
    if(statusCode == 0) {
        if(tries > 5) {
            qDebug() << "Giving up.";
        }
        tries++;
    }

    QTimer::singleShot(1000, this, SLOT(gen_next_page()));
}

void WebWatch::gotReply(QNetworkReply *reply)
{
    if(reply->header(QNetworkRequest::ContentTypeHeader).toString().contains(QString("text/html")))
    {
        qDebug() << "Got reply " + reply->url().toString() + " - " + reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toString() + " - " + reply->header(QNetworkRequest::ContentTypeHeader).toString();
    }

    if(reply->header(QNetworkRequest::ContentTypeHeader).toString().contains(QString("text/html")) && statusCode != 200) {
        statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        redirectUrl = QUrl(reply->header(QNetworkRequest::LocationHeader).toUrl());
    }
}

void WebWatch::sendData(void * buf, int size)
{
    QByteArray ba((const char *)buf, size);

    std::vector<std::string> idents;
    idents.push_back(request->conn_id);

    utils::deliver(request->sender, idents, ba, out_socket);
}

void WebWatch::sslErrors(QNetworkReply *reply, const QList<QSslError> &errors)
{
    qDebug() << errors;
    reply->ignoreSslErrors();
}

