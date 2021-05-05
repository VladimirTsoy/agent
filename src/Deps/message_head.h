#ifndef MESSAGE_HEAD_H
#define MESSAGE_HEAD_H

#include <QObject>
#include <QMap>

enum message_type {
    msg_string,     // просто строка с инфо
    msg_file,       // передаём файл на проверку
    msg_json,       // plugin_name+json
    msg_plugin_info,// инфо по плагину
    msg_cmd         // управляющая команда
};

/*!
 * \brief The msg_head struct - заголовок для пакета, передаваемого через сокет
 */
struct message_head {
    message_type type;
    quint16 sz_body;
};

#endif // MESSAGE_HEAD_H
