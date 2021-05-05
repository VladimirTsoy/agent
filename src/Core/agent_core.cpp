#include "agent_core.h"
#include <boost/exception/diagnostic_information.hpp>

void AgentCore::init()
{
    dbManager = new DBManager();
    dbManager->addDB("agent.db");
    DB = dbManager->getDB("agent.db");

    if (DB == nullptr)
        throw std::exception();

    pluginManager = new PluginManager;
    pluginManager->loadPlugins();

    fileCore = new FileCore();
    fileCore->setPluginManager(pluginManager);
    fileCore->setDBManager(dbManager);
    fileCore->prepareDB();

    netCore = new NetCore;
    netCore->setPluginManager(pluginManager);

    fileCore->setNetCore(netCore);
}

boost::thread* AgentCore::start()
{
    runPlugin("ui");

    int j;
    cout << (j = netCore->_registration()) << " reg status\n";
    while (j != 200)
    {
        j = netCore->_registration();
        boost::this_thread::sleep(boost::posix_time::seconds(1));
    }
    fileCore->start_scan();
    runPlugins();

    return netCore->start();

/* NOTE: откатил Commit 231323d2
    int reg_status;

    fileCore->start_scan();

    cout << (reg_status = netCore->_registration()) << " reg status\n";

    runPlugin("ui");
    boost::this_thread::sleep(boost::posix_time::seconds(2));

    while (reg_status != 200)
    {
        reg_status = netCore->_registration();
        boost::this_thread::sleep(boost::posix_time::seconds(5));
    }

    runPlugins();

    return netCore->start();
*/
}

http::response<http::string_body> AgentCore::sendNetRequest(const string &action, const string &data)
{
    http::response<http::string_body> res;
    printf("Send Net Request! Type: %s\n", action.c_str());
    if(action == "get_device")
        res = this->netCore->get_device_request(data);
    else if(action == "add_device")
        res = this->netCore->add_device_request(data);
    else if(action == "plugin")
        res = this->netCore->plugin_request(data);

    return res;
}


File AgentCore::checkHashInDB(std::string _hash)
{
    return this->fileCore->checkHashInDB(_hash);
}

int AgentCore::subscribePlugin(std::string pluginName)
{
    return pluginManager->subscribePlugin(pluginName);
}

map<string, short>& AgentCore::getPluginsInfo()
{
    return pluginManager->getPluginsInfo();
}

void AgentCore::notify(std::string plugName, std::string json)
{
    if (plugName == "new_file")
    {
        fileCore->addFile(json);
    }
    /*else if (plugName == "exit")
    {
        delete this;
    }*/
    else
    {
        pluginManager->notify(plugName, json);
    }
}

int AgentCore::addFileToDB(File f)
{
    return this->fileCore->addFileToDB(&f);
}
/*
int AgentCore::addDeviceToDB(const size_t id, struct Device f)
{
    table_type files;
    std::string device_id;
    std::string query;

    query = "SELECT id FROM device WHERE serial = \"" + f.serial + "\" LIMIT 1";

    int rec = this->executeSQLquery(id, query.c_str(), callbackDB, &files);
    if (rec == 0 && files.size() > 0)
    {
        device_id = files.at(0).at(0).data();
    }
    else
    {
        query = "INSERT INTO device (vid, pid, name, serial) "
                " VALUES (\"" + f.vid + "\", \"" + f.pid + "\", \"" + f.name + "\", \""
                + f.serial + "\")";

       this->executeSQLquery(id, query.c_str(), nullptr, nullptr);

        query = "SELECT id FROM device WHERE serial = \"" + f.serial + "\" LIMIT 1";
        files.clear();

       this->executeSQLquery(id, query.c_str(), callbackDB, &files);

        if (files.size() > 0)
            device_id = files.at(0).at(0).data();
    }

    query = "INSERT INTO device_block (dev_id, block_id) VALUES (\"" +
            device_id + "\", 0)";

    this->executeSQLquery(id, query.c_str(), nullptr, nullptr);

    return 0;
}
*/

std::string AgentCore::getSHA256(const std::string str)
{
    return this->fileCore->getSHA256(str);
}

std::string AgentCore::getFileStatusHash(std::string hash)
{
    // WARNING: этот метод непонятно кто дёргает - так что оставляем как для "/fileinfo"
    http::response<http::string_body> tmp = netCore->getFileStatusHash_f(hash);
    if (tmp.result_int() == 200)
        return tmp.body();

    return "";
}

std::string AgentCore::getFileStatusID(int _id)
{
    http::response<http::string_body> tmp = netCore->getFileStatusID(_id);
    if (tmp.result_int() == 200)
        return tmp.body();

    return "";
}

std::string AgentCore::sendFile( const string _file, const string _hash)
{
    http::response<http::string_body> r;

    File f;
    f.path = _file;
    f.hash = _hash;
    //this->addFileToDB(f);

    r = netCore->sendFile(_file, _hash);

    return r.body();
}

int AgentCore::executeSQLquery(const std::string &query, int(*callback)(void*,int,char**,char**), void* StructToFill)
{
    return DB->execSQLQuery_raw( query, callback, StructToFill);
}

void* AgentCore::getPluginDB(const string &path)
{
    Database *a = dbManager->getDB(path);
    if (a == nullptr)
        a = dbManager->addDB(path);

    return static_cast<void*>(a);
}

boost::thread *AgentCore::getNetThread()
{
    return netCore->_thread;
}

AgentCore::AgentCore()
{
}

AgentCore::~AgentCore()
{
    fileCore->stop_scan();

    delete fileCore;
    delete dbManager;
    delete pluginManager;
    delete netCore;
}

void AgentCore::runPlugins()
{
    std::cout << "--------------- runPlugins()---------------" << std::endl;

    for(auto &plugin : pluginManager->getPlugins())
    {
        std::cout << "**** Plugins name = " << plugin.first << std::endl;
        if(plugin.second->getName() != "ui")
        {
            cout << plugin.second->getName() << " - trying to run..." << endl;
            plugin.second->runPlugin(this);

            cout << "pluginInfo: set plugin status to \"run\"."<< endl;
            pluginManager->setPluginInfo(plugin.second->getName(), 2); // 2 - runnig
        }
    }

    std::cout << "--------------- End of runPlugins()---------------" << std::endl;
/* NOTE: откатил Commit 231323d2
    for(auto &plugin : pluginManager->getPlugins())
    {
        if(plugin.second->getName() != "ui")
        {
//            std::thread th ([&]{ plugin.second->runPlugin(this);});
            boost::thread th ([&]{ plugin.second->runPlugin(this);});
            th.detach();
        }
    }
*/
}

void AgentCore::runPlugin(std::string str)
{
    if(pluginManager->getPlugins().count(str) == 1)
    {
        pluginManager->getPlugins().at(str)->runPlugin(this);
        pluginManager->setPluginInfo(str, 2); // 2 - runnig
    }
}
