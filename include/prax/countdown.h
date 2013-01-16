#ifndef COUNTDOWN_H
#define COUNTDOWN_H

#include <QImage>
#include <QPainter>
#include <QtCore>
#include <QTimer>
#include <QtWebKit>

#include "prax/countdown.h"
#include "prax/offscreenwebpage.h"
#include "nzmqt/nzmqt.hpp"

namespace Prax {

class Countdown : public QObject {
    Q_OBJECT

    OffScreenWebPage *page;
    QWebView *view;
    QUrl redirectUrl;
    QTimer *timer;
    int tries;//failures
    int statusCode;
    QUrl pageUrl;
    int start;
    int end;
    QSize pageSize;
    QString elementId;
    nzmqt::ZMQSocket * in_socket;
    nzmqt::ZMQSocket * out_socket;

public:
    Countdown(QString in_addr, QString out_addr);

private slots:
    void gen_countdown(const QList<QByteArray> &request);
    //void gotReply(QNetworkReply *reply);
    //void sslErrors(QNetworkReply *reply, const QList<QSslError> &errors);
};

}

#endif
