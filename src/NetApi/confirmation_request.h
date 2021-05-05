#ifndef CONFIRMATIONREQUEST_H
#define CONFIRMATIONREQUEST_H


#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio.hpp>
#include <fstream>
#include <thread>
#include <iostream>

#include "request.h"
#include "../SystemInfo/system_info.h"

#include "../Settings/athena_settings.h"

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;
using namespace std;

class ConfirmationRequest : public Request
{
    void _init() override;
    void _construct_content(std::ostream&) override;

    public:

    bool construct_data() override;

    ConfirmationRequest();
};

#endif // CONFIRMATIONREQUEST_H
