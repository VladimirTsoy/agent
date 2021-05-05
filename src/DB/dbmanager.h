#ifndef DBMANAGER_H
#define DBMANAGER_H
#include "agentdb.h"


class DBManager
{
public:
    DBManager();
    ~DBManager();

    AgentDB* addDB(std::string name);
    AgentDB* getDB(std::string name);

private:
    std::map<std::string, AgentDB*> databases;
};

#endif // DBMANAGER_H
