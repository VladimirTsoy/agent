#ifndef NET_CORE_H
#define NET_CORE_H

#include <fstream>
#include <thread>
#include "request.h"
#include "registration_request.h"
#include "keepalive_request.h"
#include "confirmation_request.h"
#include "get_file_request.h"
#include "upload_file_request.h"
#include "athena_request.h"
#include "plugin_request.h"
#include "get_device_request.h"
#include "add_device_request.h"
#include "boost/uuid/uuid.hpp"
#include "../Settings/athena_settings.h"
#include <boost/thread/thread.hpp>
#include "../Plugin/plugin_manager.h"

class NetCore
{
private:
    bool _is_run;

    //thread _thread;
    AthenaRequest* ath_request;
    PluginManager* _plugin_manager;
    static void _keepalive(NetCore *obj);

    int _connection_status;
    int _registration_status;
    bool _confirmed;

public:
    http::response<http::string_body> sendFile(const std::string &_file, const std::string &_hash);
    http::response<http::string_body> getFileStatusHash_h(const std::string& _hash); ///> /hashinfo
    http::response<http::string_body> getFileStatusHash_f(const std::string& _hash); ///> /fileinfo
    http::response<http::string_body> getFileStatusID(int _id);
    http::response<http::string_body> get_device_request(const string &);
    http::response<http::string_body> add_device_request(const string &);
    http::response<http::string_body> plugin_request(const string &);

    int setPluginManager(PluginManager* _pm);
    NetCore();
    ~NetCore();

    boost::thread *_thread;

    boost::thread *start();
    boost::thread *_keepalive_start();
    int _registration();
    bool _confirmation();

    int get_registration_status() const;
    int get_connection_status() const;
    bool is_confirmed() const;
};


#endif // NET_CORE_H
