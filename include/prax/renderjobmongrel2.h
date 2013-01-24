#ifndef PRAX_RENDERJOBMONGREL2_H
#define PRAX_RENDERJOBMONGREL2_H

#include "prax/renderjob.h"
#include "prax/request.h"

namespace Prax {

class RenderJobMongrel2 : public RenderJob {
    Q_OBJECT

    Request * request;

public:
    RenderJobMongrel2(Request * req);
    Request * getRequest();
    virtual bool endConnection();
};

}

#endif
