#ifndef GET_FILE_REQUEST_H
#define GET_FILE_REQUEST_H


#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio.hpp>

#include "request.h"

#include "../Settings/athena_settings.h"

enum service_point {
    hashinfo,   // "files_get_hash_verdict\" : \"/agent_api/api/v1/hashinfo/\"
    fileinfo    // "files_get_hash_target\" : \"/agent_api/api/v1/fileinfo/\"
};

class GetFileRequest : public Request
{
private:
    int hash_type;
    string _hash;
    int _id;

    void _init() override;
    void _construct_content(std::ostream&) override;

public:
    bool construct_data() override;

    GetFileRequest(const string &, service_point);
    GetFileRequest(int);
    virtual ~GetFileRequest() override;
};
#endif
