#ifndef DEVICE_H
#define DEVICE_H

#include <string>

/*!
 * \brief The Device struct - описание подключаемого устройства
 */
struct Device
{
    std::string id;
    std::string vid;
    std::string pid;
    std::string name;
    std::string date;
    std::string serial;
    std::string block_state;
};

#endif // DEVICE_H
