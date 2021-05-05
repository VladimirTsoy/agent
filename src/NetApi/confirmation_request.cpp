#include "confirmation_request.h"
#include <version.h>

ConfirmationRequest::ConfirmationRequest() : Request()
{
    _init();
    _connect();
}

void ConfirmationRequest::_init(){
    _load_option("server", &this->_addr);
    _load_option("port", &this->_port);
    _load_option("confirmation_target", &this->_target); // api/v1/info
}

bool ConfirmationRequest::construct_data()
{
    return true;
}

void ConfirmationRequest::_construct_content(ostream &s)
{
    http::request<http::string_body> req;

    req.version(11);
    req.method(http::verb::get);
    req.set(http::field::host, this->_addr);

    req.target(this->_target);
    req.set(http::field::user_agent, string(USER_AGENT));
    req.set(http::field::content_type, "application/json"); // nn
    req.set("X-Auth-Token", this->_token);

    req.prepare_payload();

    s << req;
}
