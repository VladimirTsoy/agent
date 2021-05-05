#include "keepalive_request.h"

KeepaliveRequest::KeepaliveRequest() : Request() {
    _init();
    std::cout << "KeepAlive initialized \n";
    //Request::_io_service->reset();

    _connect();
    //cout << "Keepalive " << (this->_ssl_stream.lowest_layer().is_open() ? "connected" : "not connected") << endl;
    std::cout << "End of KeepAlive initialized \n";
}

void KeepaliveRequest::_init(){
    _load_option("server", &this->_addr);
    _load_option("port", &this->_port);
    _load_option("keepalive_target", &this->_target);
}

bool KeepaliveRequest::construct_data(){
    return true;
}

void KeepaliveRequest::_reset_connection()
{
    _disconnect();
    _connect();
}

void KeepaliveRequest::_construct_content(std::ostream &s) {
    http::request<http::empty_body> req;
    req.version(11);
    req.method(http::verb::post);
    req.set(http::field::host, this->_addr);
    req.target(this->_target);
    req.set(http::field::user_agent, string(USER_AGENT));
    req.set("X-Auth-Token", this->_token);
    req.prepare_payload();

    s << req;
}
