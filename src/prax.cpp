#include <error.h>
#include <getopt.h>
#include <iostream>
#include <QApplication>
#include <QDebug>

#include "prax/renderserver.h"
#include "prax/mongrel2.h"

void error_usage(const char * e) {
    error(1,0,"%s\nUsage: prax --addr <address> --port <port>", e);
}

int main(int argc, char *argv[])
{
    if (argc <2) error_usage("too few arguments");

    static struct option long_options[] = {
        {"base_addr" ,  1 , 0 , 'b' } , 
        {"port" ,       1 , 0 , 'p' } , 
    };

    QApplication a(argc, argv);

    QString base_addr;
    int port = -1;

    int long_index = 0;
    int opt = 0;
    while ((opt = getopt_long(
                    argc, argv,"",long_options,&long_index)) != -1) {
        switch (opt) {
            case 'b':
                base_addr = QString(optarg);
                break;

            case 'p':
                port = atoi(optarg);
                break;

            default: error_usage("unknown option");
                     break;
        }
    }

    if (base_addr.isNull() || port < 0) error_usage("please enter a base address and a port");

    if(argc - optind != 0) error_usage("bad number of options after flag parsing");

    QString addr_templ("tcp://%1:%2");

    QString addr_in = addr_templ.arg(base_addr).arg(port++);
    QString addr_out = addr_templ.arg(base_addr).arg(port++);

    Prax::Mongrel2 mongrel2(addr_in, addr_out);

    qDebug() << "binding handler to " << addr_in << " and " << addr_out;
    Prax::RenderServer renderServer(1);
    QObject::connect(&mongrel2, SIGNAL(request(Request *)), &renderServer, SLOT(request(Request *)));
    QObject::connect(&renderServer, SIGNAL(jobProgress(Request *, QByteArray)), &mongrel2, SLOT(reply(Request *, QByteArray)));
    QObject::connect(&renderServer, SIGNAL(jobDone(Request *)), &mongrel2, SLOT(endConnection(Request *)));

    return a.exec();
}
