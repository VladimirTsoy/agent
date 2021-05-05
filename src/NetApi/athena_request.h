#ifndef ATHENA_REQUEST_H
#define ATHENA_REQUEST_H

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio.hpp>
#include "request.h"
#include "../SystemInfo/system_info.h"
#include "boost/uuid/uuid.hpp"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;
using namespace std;

class AthenaRequest
{

    boost::uuids::uuid authToken;

public:

    http::response<http::string_body> request(Request *);
    http::response<http::string_body> request(Request*, std::uint64_t millis);

    AthenaRequest();

    boost::uuids::uuid getAuthToken();

    int setAuthToken(const string &token_string);
    void setRegistration();
    bool isRegistration();
};

#endif

