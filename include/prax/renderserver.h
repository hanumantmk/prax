#ifndef PRAX_RENDERSERVER_H
#define PRAX_RENDERSERVER_H

#include <QtCore>
#include <QTimer>

#include "prax/request.h"
#include "prax/render.h"
#include "nzmqt/nzmqt.hpp"

namespace Prax {

class RenderServer : public QObject {
    Q_OBJECT

    QList<Render *> idle;

    int nThreads;

public:
    RenderServer(int nThreads);

signals:
    void jobDone(Request * req);
    void jobProgress(Request * req, QByteArray bytes);

public slots:
    void request(Request * req);
    void returnJob(Render * render);
    void gatherJobProgress(RenderJob * job, QByteArray ba);
};

}

#endif
