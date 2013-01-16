#include <string>
#include <vector>
#include <sstream>
#include <json/json.h>

namespace Prax {

typedef std::pair<std::string, std::string> header;

namespace utils {

std::vector<std::string> split(const std::string& str, const std::string& sep, unsigned int count);

std::string parse_netstring(const std::string& str, std::string& rest) ;

std::vector<header> parse_json(const std::string& jsondoc);

}
}
