#ifndef REQUEST_H
#define REQUEST_H

#include <boost/beast/core.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <fstream>
#include <thread>
#include <iostream>

#include "../Settings/athena_settings.h"
#include "version.h"

#define EMPTY ""
#define VERSION(X) "AthenaAgent-"#X
#define VER_FULL(VER) VERSION(VER)
#define USER_AGENT VER_FULL(AthenaAgent_VERSION_PATCH)
#define BOUNDARY "------------------------22AthenaAgent22$"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;
using namespace std;

class Request
{
    //const boost::posix_time::seconds _connect_deadline = boost::posix_time::seconds(2);
    //const boost::posix_time::seconds _request_deadline = boost::posix_time::seconds(10);

    protected:

    boost::posix_time::seconds _request_deadline;

    AthenaSettings *_settings;

    string _addr;
    string _port;

    string _token;
    string _target;
    string _log;

    string _data;
    string _content;

    //static boost::asio::io_context* _io_service;
    boost::asio::io_context _ioc;
    boost::asio::ssl::context _ctx;
    boost::beast::ssl_stream<boost::beast::tcp_stream> _ssl_stream;
    //boost::asio::deadline_timer _dt;

    tcp::resolver *_resolver;

    virtual void _init(void);
    virtual void _construct_content(std::ostream&);

    void _load_option(string, string *);

    template<typename T>
    using is_string = std::is_constructible<std::string, T>;

    template<typename T>
    // NOTE: string plugin_name - имя плагина, то что он даёт в getName() !!
    void _load_plugin_option(string plugin_name, string _settings_key, T* source) {

        //static_assert(is_string<T>::value || std::is_integral<T>::value, "Required Type mismatched!");
        cout << "[ " << _settings_key << " ]";

        json result = this->_settings->getPluginSettingsProperty(plugin_name, _settings_key);

        if (result != json::object())
            *source = result.get<T>();
        else
            _error_log("[settings] Не найдено определение " + _settings_key);
    };

    void _connect(void);
    void _disconnect(void);

    [[ noreturn ]] void _error_log(const string) const;
    void _log_write(const string) const;
    boost::system::error_code _sock_state;

public:

    void set_token(const string &);

    virtual bool construct_data();
    string get_data();
    void set_data(const string &);

    Request();
    virtual ~Request();

    virtual http::response<http::string_body> request(void);

    std::ostringstream prepare_body();
    void prepare_body(std::ostream &s);


private:

    //void check_deadline();

    void handler_connect(const boost::system::error_code& er_c);
    void handler_handshake(const boost::system::error_code& er_c);
    void handler_write(const boost::system::error_code& er_c, size_t bytes_transfered);
    void handler_read(const boost::system::error_code& er_c, size_t bytes_read);
    void handler_wait_rd(const boost::system::error_code& er_c);
};
#endif // REQUEST_H
