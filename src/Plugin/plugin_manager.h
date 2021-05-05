#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H


#include "boost/filesystem.hpp"
#include "boost/exception/all.hpp"
#include "plugin.h"
#include <set>

#include "../Settings/athena_settings.h"

using namespace boost::filesystem;
using namespace std;

class PluginManager
{
public:

    PluginManager();
    ~PluginManager();

    Plugin* getPlugin(const string &);

    void notify(std::string, std::string);
    int subscribePlugin(std::string pluginName);

    map<string, Plugin*>& getPlugins();

    map<string, short>& getPluginsInfo(); // получить все плагины-статусы
    short getPluginInfo(string); // получить статус плагина по имени
    void setPluginInfo(string, short); // установить статус плагина

    int loadPlugins();

private:
    std::list<Plugin*>* subscribers;
    map<string, Plugin*> plugins;
    int _parsePluginsDir(path);

    AthenaSettings* settings;

    map<string, short> pluginsStatus; // name,status
    /* status:
        0  - is in list,
        1  - init,
        2  - is running (started successfully)
        -1 - deleted (init() failed)
        NOTE: сейчас есть только 1 и 2, т.к.списка нет (соотв.и -1 отпадает)
    */
};

#endif // PLUGIN_MANAGER_H
