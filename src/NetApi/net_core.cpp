#include "net_core.h"


NetCore::NetCore()
{
    this->_connection_status = 401;
    this->_registration_status = 401;
    this->ath_request = new AthenaRequest();

    this->_is_run = false;
}

NetCore::~NetCore()
{
    this->_thread->interrupt();
    this->_thread->join();
    delete ath_request;
}

http::response<http::string_body> NetCore::sendFile(const std::string& _file, const std::string& _hash)
{
    UploadFileRequest upload_file_request(_file);
    upload_file_request._hash = _hash;

    return ath_request->request(&upload_file_request);
}

http::response<http::string_body> NetCore::getFileStatusID(int id)
{
    GetFileRequest get_file_request(id);
    return ath_request->request(&get_file_request);
}

http::response<http::string_body> NetCore::getFileStatusHash_h(const std::string& _hash)
{
    GetFileRequest get_file_request(_hash, hashinfo);

    return ath_request->request(&get_file_request);
}
http::response<http::string_body> NetCore::getFileStatusHash_f(const std::string& _hash)
{
    GetFileRequest get_file_request(_hash, fileinfo);

    return ath_request->request(&get_file_request);
}

http::response<http::string_body> NetCore::get_device_request(const string &data)
{
    GetDeviceRequest get_device_request(data);

    return ath_request->request(&get_device_request);
}

http::response<http::string_body> NetCore::add_device_request(const string &data)
{
    AddDeviceRequest add_device_request(data);

    return ath_request->request(&add_device_request);
}

http::response<http::string_body> NetCore::plugin_request(const string &plugin_name)
{
    http::response<http::string_body> res;
    Plugin* p = this->_plugin_manager->getPlugin(plugin_name);
    if (p != nullptr)
    {
        PluginRequest plugin_request(p);
        res = ath_request->request(&plugin_request);
       /// if (( res.result_int() == 401 ) || ( res.result_int() == 403 ))
         ///   this->_registration();
        return res;
    }
    else
    {
        std::cout << "No plugin\n";
        return http::response<http::string_body>();
    }

}

int NetCore::setPluginManager(PluginManager *_pm)
{
    this->_plugin_manager = _pm;
    return 0;
}

int NetCore::_registration()
{
    http::response<http::string_body> result_of_request;
    RegistrationRequest registration;

    result_of_request = ath_request->request(&registration);

    switch(result_of_request.result_int()) {
    case 200:
    case 201:
        ath_request->setAuthToken(json::parse(result_of_request.body())["Token"]);
        this->_connection_status = 200;
        this->_registration_status = 200;
        _plugin_manager->notify("agent", "connect");
        break;
    case 400:
    default:
        this->_connection_status = 400;
        this->_registration_status = 400;
        _plugin_manager->notify("agent", "disconnect");
        _is_run = false;
        break;
    }

    return this->_connection_status;
}

bool NetCore::_confirmation()
{
    http::response<http::string_body> result_of_request;
    ConfirmationRequest confirmation;

    result_of_request = ath_request->request(&confirmation);

    switch(result_of_request.result_int()) {
    case 200:
    {
//       "Name": string,
//       "Uuid": string,
//       "Version": string,
//       "Token": string,
//       "ExpiredAt": "2020-09-14T08:55:07.463Z",
//       "Confirmed": bool
        std::string answer = result_of_request.body();
        json a = json::parse(answer);
        this->_confirmed = a["Confirmed"];
        break;
    }
    case 401:
    default:
        this->_confirmed = false;
        break;
    }

    return this->_confirmed;
}

int NetCore::get_registration_status() const
{
    return _registration_status;
}

int NetCore::get_connection_status() const
{
    return _connection_status;
}

bool NetCore::is_confirmed() const
{
    return _confirmed;
}

void NetCore::_keepalive(NetCore* obj)
{
    u_int result;
    std::cout << "*********** Keep Aliving************** \n";

    while(obj->_is_run)
    {
        std::cout << "NetCore::_keepalive() _is_run = " << obj->_is_run << std::endl;
        KeepaliveRequest keepalive_request;
        result = 0;
        try {
            result = obj->ath_request->request(&keepalive_request).result_int();
            //keepalive_request._reset_connection();
        } catch (...) {
            printf("No connection\n");
        }

        switch(result) {
        case 200:
            std::cout << "\t This token found ! result = 200 \n";
            obj->_connection_status = 200;

            // а вот тут теперь отдельно проверяем "подтверждён" или нет Агент
            if(!obj->_confirmation())
            {
                std::cout << "\t agent not_confirmed " << std::endl;
                obj->_plugin_manager->notify("agent", "not_confirmed");
            }
                
            else
            {
                std::cout << "\t agent confirmed " << std::endl;
                obj->_plugin_manager->notify("agent", "connect"); // confirmed
            }
                
            break;
        case 401: // такой токен не найден, пробуем зарегестироваться
            std::cout << "\t This token not found ! result = 401 \n";
            obj->_connection_status = obj->_registration();
            break;
        case 403:
            std::cout << "\t This token not found ! result = 403 \n";
        default:
            std::cout << "\t This token not found ! result = default \n";
            obj->_connection_status = 403;
            obj->_plugin_manager->notify("agent", "disconnect");
            break;
        }

        std::cout << "End of NetCore::_keepalive() _is_run = " << obj->_is_run << std::endl;
        boost::this_thread::sleep(boost::posix_time::seconds(30));      
    }

    std::cout << "*********** End of Keep Aliving ************** \n";
}

boost::thread * NetCore::_keepalive_start()
{
    std::cout << "NetCore::_keepalive_start() \n";
    boost::thread *t = new boost::thread(&NetCore::_keepalive, this);
    this->_thread = t;
    return this->_thread;
}

boost::thread * NetCore::start()
{
    printf("netcore start\n");
    if(this->_plugin_manager) {
        if(!this->_is_run)
        {
            this->_is_run = true;
            return _keepalive_start();
        }
    }
    return nullptr;
}
