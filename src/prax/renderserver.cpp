#include "prax/renderserver.h"
#include "prax/webwatch.h"

using namespace Prax;

RenderServer::RenderServer(int nThreads): nThreads(nThreads)
{
    QSize size(600, 800);

    for (int i = 0; i < nThreads; i++) {
        Render * r = new Render(size);

        connect(r, SIGNAL(jobFinished(Render *)), this, SLOT(returnJob(Render *)));
        connect(r, SIGNAL(jobProgress(RenderJob *, QByteArray)), this, SLOT(gatherJobProgress(RenderJob *, QByteArray)));

        idle.push_back(r);
    }
}

void RenderServer::request(Request * req)
{
    if (! idle.length()) {
        return;
    }

    Render * r = idle.takeLast();

    QStringList elements = req->path.split("/");

    if (elements.last() == "webwatch") {
        r->job = new WebWatch(req);
        // TODO replace this with something that picks the url out of the url
    } else {
        // TODO: do something smart here
        return;
    }

    r->run();

    //QThreadPool::globalInstance()->start(r);
}

void RenderServer::returnJob(Render * render)
{
    idle.push_back(render);

    RenderJobMongrel2 * rjm = static_cast<RenderJobMongrel2 *>(render->job);
    render->job = NULL;

    if (rjm->endConnection())
        emit jobDone(rjm->getRequest());

    delete rjm;
}

void RenderServer::gatherJobProgress(RenderJob * job, QByteArray ba)
{
    RenderJobMongrel2 * rjm = static_cast<RenderJobMongrel2 *>(job);

    emit jobProgress(rjm->getRequest(), ba);
}
