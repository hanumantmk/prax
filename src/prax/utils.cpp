#include "prax/utils.h"
#include <QHash>
#include <QString>

using namespace Prax;

QByteArray utils::parse_netstring(QByteArray & str, QByteArray & rest)
{
    QByteArray colon(":");
    QList<QByteArray> result = utils::split(str, colon, 1);

    unsigned int len = result[0].toUInt();

    rest = result[1].mid(len);
    return result[1].left(len);
}

QList<QByteArray> utils::split(QByteArray & ba, QByteArray & sep, int cnt)
{
    int old_index = 0;
    QList<QByteArray> list;

    for (int i = 0; i < cnt; i++) {
        int index = ba.indexOf(sep, i == 0 ? 0 : old_index);
        list.push_back(ba.mid(old_index, index - old_index));
        old_index = index + sep.length();
    }

    list.push_back(ba.mid(old_index));

    return list;
}

QHash<QString, QString> utils::parse_json(QString jsondoc)
{
    QHash<QString, QString> hdrs;

    json_object * jobj = json_tokener_parse(jsondoc.toUtf8().data());

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

void utils::deliver(QString & uuid, QStringList & idents, const QByteArray &data, nzmqt::ZMQSocket * socket)
{
    qDebug() << "request to deliver" << uuid << idents << data;

    assert(idents.size() <= 100);

    QString msg;

    msg += uuid + " ";

    int idents_size(idents.size()-1); // initialize with size needed for spaces
    for (int i=0; i<idents.size(); i++) {
        idents_size += idents[i].length();
    }
    msg += QString::number(idents_size) + ":";
    for (int i=0; i<idents.size(); i++) {
        msg += idents[i];
        if (i < idents.size()-1)
            msg += " ";
    }
    msg += ", ";

    QByteArray ba = msg.toAscii();
    ba.append(data);

    socket->sendMessage(ba);
}

void utils::install_support_js(OffScreenWebPage * page)
{
    QString support_js(
        "function getPos(el)"
        "{"
        "    height = el.offsetHeight;"
        "    width = el.offsetWidth;"
        "    for (var lx=0, ly=0; el != null; lx += el.offsetLeft, ly += el.offsetTop, el = el.offsetParent);"
        "    return [lx,ly,width,height];"
        "}"
        ""
        "function getLinkPos(id) {"
        "    var root = document;"
        "    var root_pos = [0,0];"
        "    if (id) {"
        "        root = document.getElementById(id);"
        "        root_pos = getPos(root);"
        "    }"
        ""
        "    var links = root.getElementsByTagName(\"a\");"
        "    var out = [];"
        ""
        "    for (var i = 0; i < links.length; i++) {"
        "        var pos = getPos(links[i]);"
        "        pos[0] -= root_pos[0];"
        "        pos[1] -= root_pos[1];"
        "        out.push([links[i].getAttribute(\"href\"), pos]);"
        "    }"
        ""
        "    return {"
        "        root : root_pos,"
        "        links : out"
        "    };"
        "}"
    );

    page->mainFrame()->evaluateJavaScript(support_js);
}
