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

    maxSize = QSize(400, 400);

    clipID = "timeline";

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
    qDebug() << "Got request";
    Request * req = new Request(msg[0]);
    qDebug() << *req;

    if (req->disconnect) {
        new_request = false;
        delete request;
        delete req;
        request = NULL;
        return;
    }

    if (req->path.find("click") != std::string::npos) {
        qDebug() << "going for click through";
        clickThrough(req);
        delete(req);
        return;
    }

    if (request) {
        endConnection();
    }

    if (pgdimage) {
        gdImageDestroy(pgdimage);
        pgdimage = NULL;
    }

    new_request = true;

    request = req;

    gen_next_page();
}

void WebWatch::clickThrough(Request * req)
{
    QString uri = req->headers["URI"];

    QRegExp rx("\\?(\\d+),(\\d+)$");

    rx.indexIn(uri);

    QStringList coor = rx.capturedTexts();

    int x = coor[1].toInt();
    int y = coor[2].toInt();

    QList<QVariant> links = clickMap.toList();

    for (int i = 0; i < links.length(); i++) {
        QList<QVariant> link = links[i].toList();

        QList<QVariant> box = link[1].toList();

        //qDebug() << "is " << x << "," << y << " in " << box;

        if (x >= box[0].toDouble() && x <= box[0].toDouble() + box[2].toDouble() &&
            y >= box[1].toDouble() && y <= box[1].toDouble() + box[3].toDouble()) {

            QString redirect = QString("HTTP/1.1 307\r\nLOCATION: %1\r\n\r\n").arg(link[0].toString());

            QByteArray ba = redirect.toAscii();

            std::vector<std::string> idents;
            idents.push_back(req->conn_id);

            utils::deliver(req->sender, idents, ba, out_socket);

            utils::deliver(req->sender, idents, NULL, out_socket);

            qDebug() << "clicking through to " << redirect;

            endConnection();
            return;
        }
    }
    endConnection();

    qDebug() << "couldn't find a link for " << x << " and " << y;
}

void WebWatch::gen_next_page()
{
    qDebug() << "generating a new page...";
    if (!request) return;
    //page->mainFrame()->load(QString("http://127.0.0.1:6767/static/foo.html"));
    page->mainFrame()->load(QString("https://twitter.com/msgpdhackday"));
}

void WebWatch::capturePage()
{
    qDebug() << "Status: " << statusCode;

    if( statusCode != 0 &&
        statusCode != 301 &&
        statusCode != 302 &&
        statusCode != 303
       ) {
        utils::install_support_js(page);

        QSize contentSize = page->mainFrame()->contentsSize();

        qDebug() << "view sizeHint: " << view->sizeHint();
        qDebug() << "page viewportSize: " << page->viewportSize();
        qDebug() << "page mainFrame contentsSize: " << page->mainFrame()->contentsSize();
        qDebug() << "page mainFrame documentElement geometry: " << page->mainFrame()->documentElement().geometry();
        qDebug() << "contentSize: " << contentSize;

        if (contentSize.height() == 0 || contentSize.width() == 0) {
            if (tries > 5) {
                endConnection();
                return;
            }
            tries++;
            QTimer::singleShot(1000, this, SLOT(gen_next_page()));
            return;
        }

        page->setViewportSize(contentSize);

        view->resize(contentSize); // for some reason this fixes garbage that would show up on the right ( past the 800 pixel mark )

        view->repaint();

        QMap<QString,QVariant> pos = page->mainFrame()->evaluateJavaScript(QString("getLinkPos(\"%1\")").arg(clipID)).toMap();
        QList<QVariant> root_pos = pos["root"].toList();
        QList<QVariant> links_pos = pos["links"].toList();

        clickMap = pos["links"];

        gdImage * gdimage = NULL;

        QRect grabRect;

        if (root_pos.length() == 4) {
            grabRect = QRect(root_pos[0].toDouble(),root_pos[1].toDouble(),root_pos[2].toDouble(),root_pos[3].toDouble());
        } else {
            grabRect = QRect(0, 0, pageSize.width(), pageSize.height());
        }

        int out_size;
        void * out_buf;

        QImage qimg = QPixmap::grabWidget(view, grabRect).toImage().convertToFormat(QImage::Format_Indexed8,0);

        QByteArray ba;
        QBuffer buffer(&ba);
        buffer.open(QIODevice::WriteOnly);
        qimg.save(&buffer, "PNG");
        buffer.close();

        if (ba.length() == 0) {
            qDebug() << "Couldn't write image";
            endConnection();
            return;
        }

        gdimage = gdImageCreateFromPngPtr(ba.length(), ba.data());

        if (new_request) {
            new_request = false;
            out_buf = gdImageGifAnimBeginPtr(gdimage, &out_size, 1, -1);

            sendData(out_buf, out_size);

            gdFree(out_buf);
        }

        //out_buf = gdImageGifAnimAddPtr(gdimage, &out_size, 1, 0, 0, 100, gdDisposalNone, pgdimage);
        out_buf = gdImageGifAnimAddPtr(gdimage, &out_size, 1, 0, 0, 100, gdDisposalNone, NULL);
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
            endConnection();
            return;
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

void WebWatch::endConnection(void)
{
    if (request) {
        sendData((void *)"", 0);
        delete request;
        request = NULL;
    }
}

