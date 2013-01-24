#ifndef PRAX_RENDERJOB_H
#define PRAX_RENDERJOB_H

#include "prax/render.h"

#include <QtCore>

namespace Prax {

class Render;

class RenderJob : public QObject {
    Q_OBJECT

public:
    virtual void init(Render * render) = 0;
    virtual QByteArray run(Render * render) = 0;

protected:
    QByteArray imageToPng(QImage &img);
    QByteArray imageToGifFrame(QImage &img);
    QByteArray gifStart(QImage &img);
    QByteArray gifEnd();
};

}

#endif
