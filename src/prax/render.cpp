#include "prax/render.h"

using namespace Prax;

Render::Render(QSize & size)
{
    setAutoDelete(false);
    pageSize = size;

    view.setPage(&page);

    connect(&page, SIGNAL(loadFinished(bool)), this, SLOT(runJob()));

    page.setViewportSize(pageSize);

    page.mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
    page.mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
}

QVariant Render::evaluateJavaScript(QString & js)
{
    return page.mainFrame()->evaluateJavaScript(js);
}

void Render::repaint()
{
    QSize contentSize = page.mainFrame()->contentsSize();

    page.setViewportSize(contentSize);

    view.resize(contentSize); // for some reason this fixes garbage that would show up on the right ( past the 800 pixel mark )

    view.repaint();
}

QImage Render::snapshot(QRect & rect)
{
    return QPixmap::grabWidget(&view, rect).toImage();
}

QImage Render::snapshot()
{
    QRect rect(0, 0, pageSize.width(), pageSize.height());

    return snapshot(rect);
}

void Render::run()
{
    job->init(this);

    exec();
}

void Render::runJob()
{
    while (true) {
        QByteArray ba = job->run(this);

        if (ba.isNull()) break;

        emit jobProgress(job, ba);
    }

    emit jobFinished(this);

    exit();
}

void Render::load(QUrl & url)
{
    view.load(url);
}
