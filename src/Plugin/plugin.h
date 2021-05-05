#ifndef PLUGIN_H
#define PLUGIN_H

#include "boost/filesystem.hpp"
#include <iostream>

#include <boost/dll/import.hpp>
#include <boost/function.hpp>
#include "agentpluginapi.h"
#include "../Core/core_api.h"

typedef boost::shared_ptr<AgentPluginApi> (fabricMethod_t)();
class Plugin
{
public:
    Plugin();

    ~Plugin();

    const std::string getName();
    //void setConnection(CoreApi*);
    void runPlugin(CoreApi*);
    const std::string getInfo();
    void setInfo(std::string, std::string);
    int init(boost::filesystem::path pathToPlugin);
    
protected:
    std::string _name;
    boost::shared_ptr<AgentPluginApi> _libAPI;
    boost::function<fabricMethod_t> _libHandler;
};

#endif // PLUGIN_H
