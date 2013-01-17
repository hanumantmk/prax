#include <string>
#include <vector>
#include <sstream>
#include <json/json.h>

#include "nzmqt/nzmqt.hpp"

namespace Prax {

typedef std::pair<std::string, std::string> header;

namespace utils {

std::vector<std::string> split(const std::string& str, const std::string& sep, unsigned int count);

std::string parse_netstring(const std::string& str, std::string& rest) ;

QHash<QString, QString> parse_json(const std::string& jsondoc);

void deliver(const std::string& uuid, const std::vector<std::string>& idents, const QByteArray &data, nzmqt::ZMQSocket * socket);

}
}
