#ifndef AGENT_CORE_H
#define AGENT_CORE_H

#include "core_api.h"
#include <iostream>

#include "../Plugin/plugin_manager.h"
#include "../Plugin/plugin.h"
#include "../NetApi/net_core.h"

#include <boost/filesystem.hpp>
#include <boost/dll/import.hpp>
#include <boost/function.hpp>
#include "../DB/dbmanager.h"
#include "../Deps/picosha2.h"
#include "file_core.h"

class AgentCore : public CoreApi
{
public:
    AgentCore();
    ~AgentCore();

    //void connectPlugins();
    void init();

    std::string getFileStatusHash(std::string hash) override;
    std::string getFileStatusID(int id) override;

    http::response<http::string_body> sendNetRequest(const string &, const string &) override;
    std::string getSHA256(const std::string) override;

    int subscribePlugin(std::string pluginName) override;
    void notify(std::string plugName, std::string status) override;

    std::list<File> getAllFilesFromDB();
    int addFileToDB(File) override ;
    //int addDeviceToDB(const size_t, struct Device) override;

    File checkHashInDB(std::string _hash) override;
    //struct Device checkSerialInDB(const size_t, const std::string &) override;

    std::string sendFile(const string _file, const string _hash = "") override;
    int executeSQLquery( const std::string &, int (*callback)(void *, int, char **, char **), void *) override;

    void* getPluginDB(const string &path) override;

    boost::thread *getNetThread();

    boost::thread *start();
    void runPlugins();
    void runPlugin(std::string str);

    map<string, short>& getPluginsInfo() override;

private:
    DBManager* dbManager;
    PluginManager* pluginManager;
    NetCore* netCore;
    AgentDB* DB;
    std::map<int, File> files;
    FileCore* fileCore;
};
#endif
