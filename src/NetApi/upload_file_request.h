#ifndef UPLOAD_FILE_REQUEST_H
#define UPLOAD_FILE_REQUEST_H

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio.hpp>

#include "request.h"

#include "../Settings/athena_settings.h"


class UploadFileRequest : public Request
{
    string _file_name;

    void _init() override;
    void _construct_content(std::ostream&) override;

public:

    bool construct_data() override;

    UploadFileRequest(const string &);

    //http::response<http::string_body> request(void) override;

    string _hash;

};

#endif
