#include "registration_request.h"
#include <version.h>

RegistrationRequest::RegistrationRequest() : Request()
{
    _init();
    _connect();
}

void RegistrationRequest::_init(){
    _load_option("server", &this->_addr);
    _load_option("port", &this->_port);
    _load_option("registration_target", &this->_target);
}

bool RegistrationRequest::construct_data() {
    this->_data = "{\"Name\": \"" + SystemInfo::machine_name + "\", " +
            "\"Uuid\": \"" + SystemInfo::uuid + "\", " +
            "\"Version\": \"" + to_string(AthenaAgent_VERSION_MAJOR) + "."
            + to_string(AthenaAgent_VERSION_MINOR) + "."
            + to_string(AthenaAgent_VERSION_PATCH) + "\" }";

    return true;
}

void RegistrationRequest::_construct_content(ostream &s)
{
    http::request<http::string_body> req;

    req.version(11);
    req.method(http::verb::post);
    req.set(http::field::host, this->_addr);
    req.target(this->_target);
    req.set(http::field::user_agent, string(USER_AGENT));
    req.set(http::field::content_type, "application/json");
    req.body() = this->_data;
    req.prepare_payload();

    s << req;
}
