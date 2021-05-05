#include "dbmanager.h"

DBManager::DBManager()
{

}

DBManager::~DBManager()
{
    std::map<std::string, AgentDB*>::iterator it;

    for (it = this->databases.begin(); it != this->databases.end(); ++it)
        delete it->second;
}


AgentDB *DBManager::addDB(std::string name)
{
    AgentDB* a = new AgentDB(name);
    this->databases[name] = a;
    return a;
}

AgentDB *DBManager::getDB(std::string name)
{
    if (this->databases.find(name) != this->databases.end())
        return this->databases[name];
    else
        return nullptr;
}
