#include "get_file_request.h"

GetFileRequest::GetFileRequest(const string &hash, service_point point) : Request()
{
    this->_hash = hash;
    this->hash_type = 1;

    _init();
    // выбираем куда обращаться
    if(point == hashinfo)
        _load_plugin_option<std::string>("file_info","files_get_hash_verdict", &this->_target);
    else // point == fileinfo
        _load_plugin_option<std::string>("file_info","files_get_hash_target", &this->_target);
    _connect();
}

GetFileRequest::GetFileRequest(int id) : Request()
{
    this->_id = id;
    this->hash_type = 0;

    _init();
    _load_plugin_option<std::string>("file_info", "files_get_id_target", &this->_target);
    _connect();
}

GetFileRequest::~GetFileRequest(){}

void GetFileRequest::_init()
{
    _load_option("server", &this->_addr);
    _load_option("port", &this->_port);
}

bool GetFileRequest::construct_data()
{
    return true;
}

void GetFileRequest::_construct_content(ostream &s)
{
    http::request<http::string_body> req;
    req.version(11);
    req.method(http::verb::get);
    req.set(http::field::host, this->_addr);

    std::string id = to_string(this->_id);
    req.target(this->_target + (this->hash_type ? this->_hash : id));
    req.set(http::field::user_agent, string(USER_AGENT));
    req.set("X-Auth-Token", this->_token);

    req.prepare_payload();

    s << req;
}
