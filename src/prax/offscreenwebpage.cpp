#include "prax/offscreenwebpage.h"

using namespace Prax;

OffScreenWebPage::OffScreenWebPage(QObject *parent) : QWebPage(parent)
{
}

void OffScreenWebPage::javaScriptAlert(QWebFrame *originatingFrame, const QString &msg)
{
}

bool OffScreenWebPage::javaScriptConfirm(QWebFrame *originatingFrame, const QString &msg)
{
    return false;
}

bool OffScreenWebPage::shouldInterruptJavaScript()
{
    return true;
}
