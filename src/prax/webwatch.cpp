#include "prax/webwatch.h"

using namespace Prax;

WebWatch::WebWatch(Request * req, QString & url) : RenderJobMongrel2(req), url(url)
{
    hasRun = false;
}

void WebWatch::init(Render * render)
{
    QUrl u(url);
    render->load(u);
}

QByteArray WebWatch::run(Render * render)
{
    if (hasRun) return QByteArray();

    hasRun = true;

    render->repaint();
    QImage image = render->snapshot();
    QByteArray bytes = imageToPng(image);

    return bytes;
}
