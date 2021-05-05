#include "get_device_request.h"

GetDeviceRequest::GetDeviceRequest(const string &serial) : Request()
{
    _init();
    _connect();

    this->_target += serial;
}

void GetDeviceRequest::_init()
{
    cout << "GetDeviceRequest::_init()\n";
    _load_option("server", &this->_addr);
    _load_option("port", &this->_port);
    _load_plugin_option<std::string>("usb_info", "get_device_target", &this->_target);
}

bool GetDeviceRequest::construct_data()
{
    return true;
}

void GetDeviceRequest::_construct_content(std::ostream &s)
{
    http::request<http::empty_body> req;
    req.version(11);
    req.method(http::verb::get);
    req.set(http::field::host, this->_addr);
    req.target(this->_target);
    req.set(http::field::user_agent, string(USER_AGENT));
    req.set("X-Auth-Token", this->_token);
    req.set(http::field::content_length, "0");

    s << req;
}
