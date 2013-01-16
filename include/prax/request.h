#include <QByteArray>
#include <QDebug>

#include "prax/utils.h"

namespace Prax {

class Request {
    public:
        std::string sender;
        std::string conn_id;
        std::string path;
        std::vector<header> headers;
        std::string body;
        bool disconnect;
        Request(QByteArray);
};

QDebug operator<<(QDebug, const Request&);

}
