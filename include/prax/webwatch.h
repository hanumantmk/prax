#ifndef PRAX_WEBWATCH_H
#define PRAX_WEBWATCH_H

#include "prax/renderjobmongrel2.h"

namespace Prax {

class WebWatch : public RenderJobMongrel2 {
    Q_OBJECT

    QString url;
    bool hasRun;

public:
    WebWatch(Request * req, QString & url);
    virtual void init(Render * render);
    virtual QByteArray run(Render * render);
};

}

#endif
