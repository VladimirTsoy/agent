#ifndef CORE_API_H
#define CORE_API_H

#include <string>
#include <list>
#include <boost/shared_ptr.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio.hpp>
#include "file.h"

class CoreApi
{

public:

    virtual std::string getFileStatusHash(std::string hash) = 0;
    virtual std::string getFileStatusID(int id) = 0;

    virtual int subscribePlugin(std::string plugName) = 0;
    virtual void notify(std::string plugName, std::string status) = 0;

    virtual std::string sendFile(const std::string _file, const std::string _hash) = 0;
    virtual boost::beast::http::response<boost::beast::http::string_body> sendNetRequest(const std::string &, const std::string &) = 0;

    virtual std::string getSHA256(const std::string _file) = 0;

    virtual int addFileToDB(File) = 0;
    virtual File checkHashInDB(std::string _hash) = 0;
    virtual int executeSQLquery( const std::string &, int (*callback)(void *, int, char **, char **), void *) = 0;

    virtual void* getPluginDB(const std::string &) = 0;


    virtual std::map<std::string, short>& getPluginsInfo() = 0;

    virtual ~CoreApi() {}
};

#endif
