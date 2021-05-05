#include "upload_file_request.h"
#include "boost/lambda/bind.hpp"
#include "boost/lambda/lambda.hpp"
#include "boost/asio/spawn.hpp"
#include "boost/beast/http/parser.hpp"
#include "boost/beast/core.hpp"

#include <codecvt>
#include <boost/filesystem/fstream.hpp>
//namespace fs = boost::filesystem;

UploadFileRequest::UploadFileRequest(const string &file_name) : Request()
{
    this->_file_name = file_name;
    _request_deadline = boost::posix_time::seconds(360);
    _init();
    _connect();
}

void UploadFileRequest::_init()
{
    _load_option("server", &this->_addr);
    _load_option("port", &this->_port);
    _load_plugin_option<std::string>("file_info", "files_upload_target", &this->_target);
}

bool UploadFileRequest::construct_data()
{
    string body;
    const string CRLF = "\r\n";
    string file_body;

    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    std::wstring wide = converter.from_bytes(this->_file_name);

    // NOTE: в пред.варианте - не открывался поток с кириллицей на Windows младше 1803
    boost::filesystem::ifstream binary_file(wide, std::ios_base::binary);
    if (binary_file){
        char ch;
        while (binary_file.get(ch))
            file_body += ch;
    }
    else{
        return false;
    }
    binary_file.close();

    std::size_t slash_found = _file_name.find_last_of("/\\");
    std::string short_filename = _file_name.substr(slash_found+1);

    body.append("--" + string(BOUNDARY) + CRLF);
    body.append("Content-Disposition: form-data; name=\"file\"; filename=\"" + short_filename + "\"" + CRLF);
    body.append("Content-Type: application/octet-stream" + CRLF);

    body.append(CRLF);
    body.append(file_body + CRLF);
    body.append("--" + string(BOUNDARY) + "--" + CRLF);
    body.append(CRLF);
    this->_data = body;
    return true;
}

void UploadFileRequest::_construct_content(std::ostream& s)
{
    string content_type_value;

    content_type_value = "multipart/form-data; boundary=";
    content_type_value += BOUNDARY;

    http::request<http::string_body> req;
    req.version(11);
    req.method(http::verb::post);
    req.target(this->_target);
    req.set(http::field::host, this->_addr);
    req.set(http::field::user_agent, string(USER_AGENT));
    req.set("X-Auth-Token", this->_token);
    std::cout << this->_hash << std::endl;
    req.set("sha256", this->_hash);
    req.set(http::field::content_type, content_type_value);
    req.body() = this->_data;
    req.prepare_payload();

    s << req;

}
