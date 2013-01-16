#include "prax/request.h"

using namespace Prax;

Request::Request(QByteArray ba)
{
    std::string result(static_cast<const char *>(ba.data()), ba.size());

    std::vector<std::string> results = utils::split(result, " ", 3);

    sender = results[0];
    conn_id = results[1];
    path = results[2];

    std::string unparsed_headers = utils::parse_netstring(results[3], body);

    headers = utils::parse_json(unparsed_headers);

    std::string ign;

    body = utils::parse_netstring(body, ign);

    //check disconnect flag
    disconnect = false;
    for (std::vector<header>::const_iterator it = headers.begin();
                    it != headers.end(); ++it) {
            if (it->first == "METHOD" && it->second == "JSON" &&
                            body == "{\"type\":\"disconnect\"}") {
                    disconnect = true;
                    break;
            }
    }
}

QDebug operator<<(QDebug dbg, const Request& req)
{
    dbg << "{ " <<
        "sender : " << req.sender.c_str() << "," <<
        "conn_id : " << req.conn_id.c_str() << "," <<
        "body : " << req.body.c_str() << "," <<
        "disconnect : " << req.disconnect <<
    "}";

    return dbg;
}
