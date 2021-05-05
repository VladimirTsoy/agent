#ifndef AGENTPLUGINAPI_H
#define AGENTPLUGINAPI_H

#include <string>
#include <boost/shared_ptr.hpp>

class BOOST_SYMBOL_VISIBLE AgentPluginApi
{
public:
    virtual const std::string getName() const = 0;

    /*!
     * \brief setConnection
     * void* core_api - Указатель на Core API!!!
     */
    virtual void setConnection(void *core_api) = 0;
    virtual void runPlugin() = 0;

    virtual void stopPlugin() = 0;

    virtual void setInfo(std::string pluginName, std::string json) = 0;
    virtual int getInfo(std::string *a) = 0;

    virtual ~AgentPluginApi() {}

};

#endif // AGENTPLUGINAPI_H
