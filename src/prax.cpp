#include <error.h>
#include <getopt.h>
#include <iostream>
#include <QApplication>
#include <QDebug>

#include "prax/countdown.h"

void error_usage(const char * e) {
    error(1,0,"%s\nUsage: prax --addr <address>", e);
}

int main(int argc, char *argv[])
{
    if (argc <2) error_usage("too few arguments");

    static struct option long_options[] = {
        {"in_addr" ,  1 , 0 , 'i' } , 
        {"out_addr" , 1 , 0 , 'o' } , 
    };

    QApplication a(argc, argv);

    QString in_addr;
    QString out_addr;

    int long_index = 0;
    int opt = 0;
    while ((opt = getopt_long(
                    argc, argv,"",long_options,&long_index)) != -1) {
        switch (opt) {
            case 'i':
                in_addr = QString(optarg);
                break;

            case 'o':
                out_addr = QString(optarg);
                break;

            default: error_usage("unknown option");
                     break;
        }
    }

    if (in_addr.isNull() || out_addr.isNull()) error_usage("please enter an in address and an out address");

    if(argc - optind != 0) error_usage("bad number of options after flag parsing");

    Prax::Countdown countdown(in_addr, out_addr);

    return a.exec();
}
