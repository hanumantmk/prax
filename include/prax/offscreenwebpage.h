#ifndef OFFSCREENWEBPAGE_H
#define OFFSCREENWEBPAGE_H

#include <QtCore>
#include <QtWebKit>
#include <QWebPage>

namespace Prax {

class OffScreenWebPage : public QWebPage {

public:
    OffScreenWebPage(QObject *parent = 0);

protected:
    void javaScriptAlert(QWebFrame *originatingFrame, const QString &msg);
    bool javaScriptConfirm(QWebFrame *originatingFrame, const QString &msg);

public slots:
    bool shouldInterruptJavaScript();
};

}

#endif
