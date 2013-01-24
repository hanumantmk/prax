#include <string>
#include <vector>
#include <sstream>
#include <json/json.h>

#include "nzmqt/nzmqt.hpp"

#include "prax/offscreenwebpage.h"

namespace Prax {

namespace utils {

QByteArray parse_netstring(QByteArray & str, QByteArray & rest);

QHash<QString, QString> parse_json(QString jsondoc);

void deliver(QString & uuid, QStringList & idents, const QByteArray &data, nzmqt::ZMQSocket * socket);

void install_support_js(OffScreenWebPage * page);

QList<QByteArray> split(QByteArray & ba, QByteArray & sep, int cnt);

}
}
