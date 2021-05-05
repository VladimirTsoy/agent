#ifndef AGENTDB_H
#define AGENTDB_H

#include "../Deps/sqlite3.h"
#include <stdio.h>
#include <vector>
#include <map>
#include <string>
#include "database.h"
#include <boost/filesystem.hpp>

class AgentDB : public Database
{

public:
    AgentDB(const std::string &path);
    ~AgentDB() override;

    int execSQLQuery( const std::string &) override;
    int execSQLQuery_raw( const std::string &, int (*callback)(void *, int, char **, char **), void *) override;

private:
//    sqlite3 *connections[100];

    sqlite3 *connection;

    std::string GetUserAppsDir(const std::string &path);
};

#endif // AGENTDB_H
