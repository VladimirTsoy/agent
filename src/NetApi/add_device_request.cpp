#include "add_device_request.h"

AddDeviceRequest::AddDeviceRequest(const string &data) : Request()
{
    _init();
    _connect();

    this->_data = data;
}

void AddDeviceRequest::_init()
{
    _load_option("server", &this->_addr);
    _load_option("port", &this->_port);
    _load_plugin_option<std::string>("usb_info", "add_device_target", &this->_target);
}

bool AddDeviceRequest::construct_data()
{
    return true;
}

void AddDeviceRequest::_construct_content(std::ostream &s)
{
    http::request<http::string_body> req;
    req.version(11);
    req.method(http::verb::post);
    req.set(http::field::host, this->_addr);
    req.target(this->_target);
    req.set(http::field::content_type, "application/json");
    req.set(http::field::user_agent, string(USER_AGENT));
    req.set("X-Auth-Token", this->_token);
    req.body() = this->_data;
    req.prepare_payload();

    s << req;
}
