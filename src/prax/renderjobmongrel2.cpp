#include "prax/renderjobmongrel2.h"

using namespace Prax;

RenderJobMongrel2::RenderJobMongrel2(Request * req)
{
    request = req;
}

Request * RenderJobMongrel2::getRequest()
{
    return request;
}

bool RenderJobMongrel2::endConnection()
{
    return true;
}
