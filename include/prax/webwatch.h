#ifndef WEBWATCH_H
#define WEBWATCH_H

#include <QImage>
#include <QPainter>
#include <QtCore>
#include <QTimer>
#include <QtWebKit>
#include <gd.h>

#include "prax/request.h"
#include "prax/offscreenwebpage.h"
#include "nzmqt/nzmqt.hpp"

namespace Prax {

class WebWatch : public QObject {
    Q_OBJECT

    OffScreenWebPage *page;
    QWebView *view;
    QUrl redirectUrl;
    QTimer *timer;
    int tries;//failures
    int statusCode;
    QUrl pageUrl;
    QSize pageSize;
    QString elementId;
    nzmqt::ZMQSocket * in_socket;
    nzmqt::ZMQSocket * out_socket;
    Request * request;
    bool new_request;
    gdImage * pgdimage;
    QString clipID;
    QVariant clickMap;

public:
    WebWatch(QString in_addr, QString out_addr);

private:
    void sendData(void * buf, int size);
    void clickThrough(Request * req);

private slots:
    void gen_page(const QList<QByteArray> &request);
    void gen_next_page();
    void capturePage();
    void gotReply(QNetworkReply *reply);
    void sslErrors(QNetworkReply *reply, const QList<QSslError> &errors);
};

}

#endif
