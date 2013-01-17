#include "prax/utils.h"
#include <QHash>
#include <QString>

using namespace Prax;

std::vector<std::string> utils::split(const std::string& str, const std::string& sep, unsigned int count) {
	std::vector<std::string> result;
	std::string::size_type last_pos = str.find_first_not_of(sep, 0);
	std::string::size_type pos = str.find_first_of(sep, last_pos);
	int i = count;

	while (std::string::npos != pos || std::string::npos != last_pos) {
		result.push_back(str.substr(last_pos, pos - last_pos));
		last_pos = str.find_first_not_of(sep, pos);
		pos = str.find_first_of(sep, last_pos);
		if (count > 0) {
			i--;
			if (i==0) {
				result.push_back(str.substr(last_pos, str.length() - last_pos));
				break;
			}
		}
	}

	return result;
}

std::string utils::parse_netstring(const std::string& str, std::string& rest) {
	std::vector<std::string> result = utils::split(str, ":", 1);
	std::istringstream is(result[0]);
	unsigned int len;
	is >> len;
	rest = result[1].substr(len+1, result[1].length() - len);
	return result[1].substr(0, len);
}

QHash<QString, QString> utils::parse_json(const std::string& jsondoc) {
	QHash<QString, QString> hdrs;

	json_object * jobj = json_tokener_parse(jsondoc.c_str());

	if (jobj && json_object_is_type(jobj, json_type_object)) {
		json_object_object_foreach(jobj, key, value) {
			if (key && value && json_object_is_type(value, json_type_string)) {
				hdrs[key] = QString(json_object_get_string(value));
			}
		}
	}

	json_object_put(jobj); // free json object

	return hdrs;
}

void utils::deliver(const std::string& uuid, const std::vector<std::string>& idents, const QByteArray &data, nzmqt::ZMQSocket * socket) {
	assert(idents.size() <= 100);
	std::ostringstream msg;
	msg << uuid << " ";

	size_t idents_size(idents.size()-1); // initialize with size needed for spaces
	for (size_t i=0; i<idents.size(); i++) {
		idents_size += idents[i].size();
	}
	msg << idents_size << ":";
	for (size_t i=0; i<idents.size(); i++) {
		msg << idents[i];
		if (i < idents.size()-1)
			msg << " ";
	}
	msg << ", ";

        std::string str = msg.str();

        QByteArray ba(str.data(), str.length());
        ba.append(data);

        socket->sendMessage(ba);
}
