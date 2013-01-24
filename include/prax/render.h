#ifndef PRAX_RENDER_H
#define PRAX_RENDER_H

#include <QImage>
#include <QPainter>
#include <QtCore>
#include <QTimer>
#include <QtWebKit>
#include <gd.h>

#include "prax/renderjob.h"
#include "prax/offscreenwebpage.h"

namespace Prax {

class RenderJob;

class Render : public QEventLoop, public QRunnable {
    Q_OBJECT

    OffScreenWebPage page;
    QWebView view;
    QSize pageSize;

public:
    Render(QSize & size);

    RenderJob * job;

    QVariant evaluateJavaScript(QString &js);
    void repaint();
    QImage snapshot();
    QImage snapshot(QRect & rect);
    void load(QUrl & url);

    virtual void run();

signals:
    void jobProgress(RenderJob * job, QByteArray ba);
    void jobFinished(Render * render);

private slots:
    void runJob();
};

}

#endif
