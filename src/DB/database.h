#ifndef DATABASE_H
#define DATABASE_H

#include <string>

class Database
{
public:

    virtual ~Database() {}

    virtual int execSQLQuery( const std::string &) = 0;
    virtual int execSQLQuery_raw( const std::string &, int (*callback)(void *, int, char **, char **), void *) = 0;

};

#endif // DATABASE_H
