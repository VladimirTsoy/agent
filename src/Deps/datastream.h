#ifndef DATASTREAM_H
#define DATASTREAM_H

#include <QDataStream>

#include "message_head.h"
#include "Core/file.h"

// Перегружаем операторы для упрощения записи/считывания при работе с сокетом

QDataStream& operator<<(QDataStream& out, const message_head& head) {
    out << head.type
        << head.sz_body;
    return out;
}
QDataStream& operator>>(QDataStream& in, message_head& head) {
    in >> head.type
       >> head.sz_body;
    return in;
}

QDataStream& operator<<(QDataStream& out,const File& file) {
    out << file.id;
    out << QString::fromStdString(file.name);
    out << QString::fromStdString(file.path);
    out << QString::fromStdString(file.hash);
    out << file.status;
    out << QString::fromStdString(file.verdict);
    out << QString::fromStdString(file.time);
    out << file.webid;
    return out;
}
QDataStream& operator>>(QDataStream& in, File& file) {
    QString str_tmp;

    in >> file.id;
    in >> str_tmp; file.name = str_tmp.toStdString();
    in >> str_tmp; file.path = str_tmp.toStdString();
    in >> str_tmp; file.hash = str_tmp.toStdString();
    in >> file.status;
    in >> str_tmp; file.verdict = str_tmp.toStdString();
    in >> str_tmp; file.time = str_tmp.toStdString();
    in >> file.webid;

    return in;
}

#endif // DATASTREAM_H
