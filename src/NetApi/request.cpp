#include "request.h"
#include "boost/lambda/bind.hpp"
#include "boost/lambda/lambda.hpp"
#include "boost/asio/spawn.hpp"
#include "boost/beast/http/parser.hpp"
#include "boost/beast/core.hpp"

using namespace boost::lambda;
using boost::lambda::bind;

//boost::asio::io_service* Request::_io_service = new boost::asio::io_context();



Request::Request()
    : _ctx(boost::asio::ssl::context::method::sslv23_client), _sock_state(boost::system::error_code{}), /*_io_service(new boost::asio::io_context()),*/
    _ioc(), _ssl_stream(_ioc, _ctx), _request_deadline(boost::posix_time::seconds(60)) /*_dt(_ioc, boost::posix_time::ptime(boost::posix_time::pos_infin))*/
{
    std::cout << "Request incoming...\n";
    this->_settings = AthenaSettings::getSettings();

    this->_content.clear();
    this->_token.clear();
    _init();
    std::cout << "End of request \n";

    //check_deadline();
}

Request::~Request() {
    _disconnect();

    delete this->_resolver;
}

void Request::_init(){
    _load_option("log", &this->_log);
}

void Request::_load_option(string key, string *source) {
    json answer = _settings->getProperty(key);

    if(answer == json::object())
        _error_log("[settings] Не найдено определение " + key);
    else *source = answer.get<std::string>();
}

void Request::_connect(void) {
    cout << "Connecting...\n";

    this->_resolver = new tcp::resolver(_ioc);


    boost::asio::spawn(_ioc,
        [&](boost::asio::yield_context yield)
        {
            try {
                auto const results = (*this->_resolver).resolve(this->_addr, this->_port);
                get_lowest_layer(_ssl_stream).expires_after(std::chrono::seconds(10));

                // `tcp_stream` range connect algorithms are member functions, unlike net::
                get_lowest_layer(_ssl_stream).async_connect(results, yield[_sock_state]);
                if (_sock_state)
                    return;

                // Perform the TLS handshake
                _ssl_stream.async_handshake(net::ssl::stream_base::client, yield[_sock_state]);
                if (_sock_state)
                    return;
            }
            catch (const boost::system::system_error& ec)
            {
                cout << "Connect error: " << ec.what() << endl;
            }
        });

    _ioc.run();
    _ioc.reset();

    get_lowest_layer(_ssl_stream).expires_never();

    if (_sock_state)
    {
        cout << "Connect operation failed: " << _sock_state.message() << " [ " << _sock_state.value() << " ]\n";
    }
    else
    {
        cout << "Connection success\n";
    }
}

void Request::_disconnect(void) {

    beast::error_code ec;
    cout << "disconnecting...\n";
    get_lowest_layer(this->_ssl_stream).socket().cancel(ec);
    _ssl_stream.shutdown(_sock_state);

    if (_sock_state == net::ssl::error::stream_truncated)
        _sock_state = {};
    else if (_sock_state)
        return;
    get_lowest_layer(this->_ssl_stream).socket().close(ec);


    if(ec && ec != beast::errc::not_connected)
        throw beast::system_error { ec };
}

void Request::_error_log (const string message) const{
    if(this->_log != "false") _log_write(message);
    printf("%s\n", message.c_str());
    exit(1);
}

void Request::_log_write(const string text) const{
    if(this->_log != "false"){
        std::ofstream outfile ("log", std::ios_base::app);
        outfile << text << std::endl;
        outfile.close();
    }
}


void Request::_construct_content(ostream &){}

void Request::set_token(const string &new_token){
    this->_token = new_token;
}

void Request::prepare_body(std::ostream &s) {
    _construct_content(s);
}

//void Request::check_deadline()
//{
//    if (_dt.expires_at() <= boost::asio::deadline_timer::traits_type::now())
//    {
//
//        boost::system::error_code ignored_ec;
//        _ssl_stream.lowest_layer().close(ignored_ec);
//
//        _dt.expires_at(boost::posix_time::pos_infin);
//    }
//
//    // Put the actor back to sleep.
//    _dt.async_wait(boost::lambda::bind(&Request::check_deadline, this));
//}

void Request::handler_connect(const boost::system::error_code& er_c)
{
    if (!er_c || er_c == boost::asio::error::would_block)
    {
        cout << "socket connected succesfully\n";
        const_cast<boost::system::error_code&>(er_c) = boost::asio::error::would_block;
        _ssl_stream.async_handshake(boost::asio::ssl::stream_base::handshake_type::client,
            boost::lambda::bind(&Request::handler_handshake, this, var(this->_sock_state) = _1));
    }
    else {
        cout << "socket unable to endpoint: " << er_c.message() << " [ " << er_c.value() << " ]\n";
    }
}

void Request::handler_handshake(const boost::system::error_code& er_c)
{
    if (!er_c || er_c == boost::asio::error::would_block)
    {
        cout << "handshaked succesfully\n";
        if(er_c)
            const_cast<boost::system::error_code&>(er_c) = boost::system::error_code();
    }
    else {
        cout << "socket unable to perform a handshake operation: " << er_c.message() << " [ " << er_c.value() << " ]\n";
    }
}

void Request::handler_write(const boost::system::error_code& er_c, size_t bytes_transfered)
{
    if (!er_c || er_c == boost::asio::error::would_block)
    {
        cout << "Written " << bytes_transfered << " bytes\n";
           //const_cast<boost::system::error_code&>(er_c) = boost::asio::error::would_block;
    }
    else {
        cout << "socket error: attempt to write some data: " << er_c.message() << " [ " << er_c.value() << " ]\n";
    }
}

void Request::handler_read(const boost::system::error_code& er_c, size_t bytes_read)
{
    if (!er_c || er_c == boost::asio::error::would_block)
    {
        cout << "Read " << bytes_read << " bytes\n";
        //const_cast<boost::system::error_code&>(er_c) = boost::asio::error::would_block;
    }
    else {
        cout << "socket error: attempt to read some data: "
            << er_c.message() << " [ " << er_c.value() << " ]\n";
    }
}

void Request::handler_wait_rd(const boost::system::error_code& er_c)
{
    if (!er_c || er_c == boost::asio::error::would_block)
    {
        cout << "rd wait handler!\n";
    }
    else {
        cout << "socket wait error: " << er_c.message() << " [ " << er_c.value() << " ]\n";
    }
}

bool Request::construct_data() { return true; }

string Request::get_data() { return this->_data; }
void Request::set_data(const string &data)
{
    this->_data = data;
}

http::response<http::string_body> Request::request(void) {


    if (!get_lowest_layer(this->_ssl_stream).socket().is_open() || _sock_state)
    {
            cout << "Request error " << _sock_state.message() << "\n";
            return http::response<http::string_body>(http::status::not_acceptable, 11);
    }


    beast::flat_buffer buffer, bfreq;
    http::response<http::string_body> res(http::status::no_content, 11);

    boost::asio::streambuf tmpbuf;
    std::ostream request_buf(&tmpbuf);

    try {

        _construct_content(request_buf);

        // get buffer size
        size_t nBufferSize = boost::asio::buffer_size(tmpbuf.data());
        size_t nBufferTransfered = 0;
        boost::asio::streambuf::const_buffers_type constBuffer = tmpbuf.data();

        size_t data_written = 0;
        boost::system::error_code ec;

        boost::asio::spawn(_ioc,
            [&](boost::asio::yield_context yield)
            {

                try {
                    cout << "writing\n";

                    get_lowest_layer(_ssl_stream).expires_after(std::chrono::seconds(_request_deadline.total_seconds()));

                    //http::async_write(_ssl_stream, req, yield[_sock_state]);
                    net::async_write(_ssl_stream, constBuffer, yield[_sock_state]);
                    //_ssl_stream.async_write_some(constBuffer, [&]() {});

                    if (_sock_state && _sock_state != net::ssl::error::stream_truncated)
                    {
                        return;
                    }


                    cout << "reading\n";
                    http::async_read(_ssl_stream, buffer, res, yield[_sock_state]);

                    if (_sock_state == boost::asio::error::eof || _sock_state == net::ssl::error::stream_truncated)
                        _sock_state = {};
                    else if (_sock_state)
                        return;
                }
                catch (const boost::system::system_error& ec)
                {
                    cout << "readwrite error: " << ec.what() << endl;
                }
                //// Perform the TLS closing handshake
                //_ssl_stream.async_shutdown(yield[_sock_state]);
                //if (_sock_state == net::ssl::error::stream_truncated)
                //    _sock_state = {};
                //else if (_sock_state)
                //    return;
            });
        _ioc.run();
        _ioc.reset();

        get_lowest_layer(_ssl_stream).expires_never();

        if (_sock_state)
        {
            cout << "Error during writing or reading contents: " << _sock_state.message() << " [ " << _sock_state.value() << " ]\n";
        }

        std::cout << "I send this: " << std::endl;
        std::cout << "****" << std::endl;
        std::cout << (const char*)constBuffer.data() << std::endl;
        std::cout << "****" << std::endl << std::endl;//res.base() << res.body() << std::endl << std::endl;

    } catch(exception &e){
        printf("Request exception: %s\n", e.what());
        //_error_log("[upload_file] не удалось подключиться к серверу");
    }

    return res;
}
