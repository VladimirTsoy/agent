#include "athena_request.h"
#include <iostream>

AthenaRequest::AthenaRequest()
{
    boost::uuids::nil_generator gen;
    authToken = gen();
}

boost::uuids::uuid AthenaRequest::getAuthToken()
{
    return this->authToken;
}


int AthenaRequest::setAuthToken(const string &token_string)
{
    boost::uuids::string_generator gen;
    try
    {
        this->authToken = gen(token_string);
    }
    catch (...)
    {
        return -1;
    }
    return 0;
}

http::response<http::string_body> AthenaRequest::request(Request *request)
{
    try
    {
        http::response<http::string_body> r;

        request->set_token(to_string(this->getAuthToken()));

        /*
         *  Definition of data to send. (Without http headers).
         *  If there is data to send - send a request
         */

        if(request->construct_data())
        {
            r = request->request();

            std::cout
                    << "I get this: " << std::endl;
            std::cout << "****" << std::endl;
            std::cout << r << endl;
            std::cout << "****" << std::endl;
        }

        return r;
    }
    catch (...)
    {
        std::cout << "Request error!" << std::endl;
    }

}
