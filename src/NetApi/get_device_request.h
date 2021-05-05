#ifndef GET_DEVICE_REQUEST_H
#define GET_DEVICE_REQUEST_H


#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio.hpp>

#include "request.h"

#include "../Settings/athena_settings.h"

class GetDeviceRequest : public Request
{
    void _init() override;
    void _construct_content(std::ostream&) override;

    public:

    bool construct_data() override;

    GetDeviceRequest(const string &);
};

#endif
