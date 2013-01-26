#include "prax/webwatch.h"

using namespace Prax;

WebWatch::WebWatch(Request * req) : RenderJobMongrel2(req)
{
    hasRun = false;

    url = req->urlParams["url"];
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
