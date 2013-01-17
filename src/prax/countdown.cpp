#include <gd.h>

#include "prax/request.h"
#include "prax/countdown.h"

using namespace Prax;

Countdown::Countdown(QString in_addr, QString out_addr)
{
    page = new OffScreenWebPage;
    view = new QWebView;
    view->setPage(page);

    start = 60;
    end = 1;

    qDebug() << "Binding to " << in_addr << " and " << out_addr;

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
    connect(in_socket, SIGNAL(messageReceived(const QList<QByteArray> &)), SLOT(gen_countdown(const QList<QByteArray> &)));

    out_socket = context->createSocket(nzmqt::ZMQSocket::TYP_PUB);
    out_socket->connectTo(out_addr);
}

void Countdown::gen_countdown(const QList<QByteArray> &msg)
{
    qDebug() << "Got request";
    Request * req = new Request(msg[0]);
    qDebug() << *req;

    request = req;

    page->mainFrame()->load(QString("http://127.0.0.1:6767/static/banner.html"));
}

void Countdown::capturePage()
{
    qDebug() << "Status: " << statusCode;

    if( statusCode != 0 &&
        statusCode != 301 &&
        statusCode != 302 &&
        statusCode != 303
       ) {

        QString SupportJS = QString(
        "function setTime(hours,minutes,seconds) {\n"
        "    document.getElementById('HH').innerHTML=hours;\n"
        "    document.getElementById('MM').innerHTML=minutes;\n"
        "    document.getElementById('SS').innerHTML=seconds;\n"
        "}\n"
        "\n"
        "function getPos(id) {\n"
        "    el = document.getElementById(id);\n"
        "    height = el.offsetHeight;\n"
        "    width = el.offsetWidth;\n"
        "    for (var lx=0, ly=0;\n"
        "         el != null;\n"
        "         lx += el.offsetLeft, ly += el.offsetTop, el = el.offsetParent);\n"
        "    return [lx,ly,width,height];\n"
        "}\n"
        );

        QVariant r = page->mainFrame()->evaluateJavaScript(SupportJS);

        QSize contentSize = page->mainFrame()->contentsSize();

        qDebug() << "view sizeHint: " << view->sizeHint();
        qDebug() << "page viewportSize: " << page->viewportSize();
        qDebug() << "page mainFrame contentsSize: " << page->mainFrame()->contentsSize();
        qDebug() << "page mainFrame documentElement geometry: " << page->mainFrame()->documentElement().geometry();
        qDebug() << "contentSize: " << contentSize;
        qDebug() << "start: " << start;
        qDebug() << "end: " << end;


        page->setViewportSize(contentSize);

        view->resize(contentSize); // for some reason this fixes garbage that would show up on the right ( past the 800 pixel mark )

        view->repaint();

        QString setTimeJS = QString();
        QString getPosJS = QString();

        gdImage * pgdimage = NULL;
        gdImage * gdimage = NULL;

        QRect grabRect;

        grabRect = QRect(0, 0, pageSize.width(), pageSize.height());

        int motion;
        if (start>end) motion=-1;
        else motion=1;

        int out_size;
        void * out_buf;

        for ( int s=start; motion*s <= motion*end; s+=motion ) {

            int hours   = s / ( 60 * 60 );
            int minutes = ( s / 60 ) % 60;
            int seconds = s % 60;

            setTimeJS.sprintf("setTime('%02d','%02d','%02d');",hours,minutes,seconds);
            page->mainFrame()->evaluateJavaScript(setTimeJS);

            if ( ! elementId.isNull() ) {
                QList<QVariant> geometryArray = page->mainFrame()->evaluateJavaScript(getPosJS).toList();
                /* being that we really don't know if the javascript is loaded,
                 * we should be more careful that this all works */

                grabRect = QRect(geometryArray[0].toDouble(),geometryArray[1].toDouble(),geometryArray[2].toDouble(),geometryArray[3].toDouble());
            }

            QImage qimg = QPixmap::grabWidget(view, grabRect).toImage().convertToFormat(QImage::Format_Indexed8,0);

            QByteArray ba;
            QBuffer buffer(&ba);
            buffer.open(QIODevice::WriteOnly);
            qimg.save(&buffer, "PNG");
            buffer.close();

            gdimage = gdImageCreateFromPngPtr(ba.length(), ba.data());

            if (s == start) {
                out_buf = gdImageGifAnimBeginPtr(gdimage, &out_size, 1, -1);

                sendData(out_buf, out_size);

                gdFree(out_buf);
            }

            //int save_each_frame = 0;
            //if (save_each_frame) {
            //    QString extraGifFilename = QString();
            //    extraGifFilename.sprintf("%d_%s",s,lOutputFilename.toUtf8().constData());
            //    FILE * extraGifFile = fopen(extraGifFilename.toUtf8().constData(),"w+");
            //    gdImageGif(gdimage,extraGifFile);
            //    fclose(extraGifFile);
            //}
        
            out_buf = gdImageGifAnimAddPtr(gdimage, &out_size, 1, 0, 0, 100, gdDisposalNone, pgdimage);
            sendData(out_buf, out_size);
            gdFree(out_buf);

            if ( pgdimage ) gdImageDestroy(pgdimage);
            pgdimage = gdimage;
        }
        out_buf = gdImageGifAnimEndPtr(&out_size);
        sendData(out_buf, out_size);
        sendData((void *)"", 0);
        gdFree(out_buf);

        gdImageDestroy(gdimage);
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

    delete request;
}

void Countdown::gotReply(QNetworkReply *reply)
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

void Countdown::sendData(void * buf, int size)
{
    QByteArray ba((const char *)buf, size);

    std::vector<std::string> idents;
    idents.push_back(request->conn_id);

    utils::deliver(request->sender, idents, ba, out_socket);
}

void Countdown::sslErrors(QNetworkReply *reply, const QList<QSslError> &errors)
{
    qDebug() << errors;
    reply->ignoreSslErrors();
}

