#include "plugin_request.h"

PluginRequest::PluginRequest(Plugin *plugin) : Request()
{
    this->_plugin = plugin;

    _init();
    _connect();
}

void PluginRequest::_init(){
    _load_option("server", &this->_addr);
    _load_option("port", &this->_port);
    _load_plugin_option<std::string>(this->_plugin->getName(), "target", &this->_target);
}

bool PluginRequest::construct_data(){

    this->_data = this->_plugin->getInfo();

    return !this->_data.empty();
}

void PluginRequest::_construct_content(ostream &s)
{
    http::request<http::string_body> req;

    req.version(11);
    req.method(http::verb::post);
    req.set(http::field::host, this->_addr);
    req.target(this->_target);
    req.set(http::field::user_agent, string(USER_AGENT));
    req.set("X-Auth-Token", this->_token);
    req.set(http::field::content_type, "application/json");
    req.body() = this->_data;
    req.prepare_payload();

    s << req;
}
