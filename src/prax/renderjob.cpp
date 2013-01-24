#include "prax/renderjob.h"

using namespace Prax;

QByteArray RenderJob::imageToPng(QImage & img)
{
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    img.save(&buffer, "PNG");
    buffer.close();

    return ba;
}

QByteArray RenderJob::gifStart(QImage &img)
{
    QByteArray pngData = imageToPng(img);

    gdImagePtr gdimage = gdImageCreateFromPngPtr(pngData.length(), pngData.data());

    void * buf;
    int size;

    buf = gdImageGifAnimBeginPtr(gdimage, &size, 1, -1);

    QByteArray ba((const char *)buf, size);

    gdFree(buf);

    return ba;
}

QByteArray RenderJob::gifEnd()
{
    void * buf;
    int size;

    buf = gdImageGifAnimEndPtr(&size);

    QByteArray ba((const char *)buf, size);

    gdFree(buf);

    return ba;
}

QByteArray RenderJob::imageToGifFrame(QImage &img)
{
    QByteArray pngData = imageToPng(img);

    gdImagePtr gdimage = gdImageCreateFromPngPtr(pngData.length(), pngData.data());

    void * buf;
    int size;

    buf = gdImageGifAnimAddPtr(gdimage, &size, 1, 0, 0, 100, gdDisposalNone, NULL);

    QByteArray ba((const char *)buf, size);

    gdFree(buf);

    return ba;
}
