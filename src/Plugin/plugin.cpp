#include "plugin.h"
#include <boost/exception/diagnostic_information.hpp>
#include <boost/exception_ptr.hpp>

Plugin::Plugin()
{
}

Plugin::~Plugin()
{
    printf("Plugin destructor!\n");
    this->_libHandler.clear();
    //this->_libAPI.reset();
    this->_name.clear();
}

/*!
 * \brief Plugin::init
 * Инициализация плагина
 * \param pathToPlugin
 * Путь к файлу плагина
 * \return
 * -1: ошибка
 * 0: Успех
 */
int Plugin::init(boost::filesystem::path pathToPlugin) 
{

    if (!is_regular_file(pathToPlugin)) 
	{
        return -1;
    }
    try 
	{
        this->_libHandler = boost::dll::import_alias<fabricMethod_t>(pathToPlugin, "create", boost::dll::load_mode::append_decorations);
        this->_libAPI = this->_libHandler();
        this->_name = this->_libAPI->getName();

    } catch (boost::exception &e)
	{
        std::cerr << boost::diagnostic_information(e);
        printf("Error init plugin %s\n", pathToPlugin.string().c_str());
        return -1;
    }
    catch (std::exception s) {
        printf("Error init plugin %s\n", pathToPlugin.string().c_str());
        std::cerr << s.what();
        return -1;
    }

    printf("Plugin %s successfully initialized!\n", this->_name.c_str());

    return 0;
}

const std::string Plugin::getInfo() 
{
    std::string result;

    int len = 0;
    len = this->_libAPI->getInfo(&result);
    return result;
}

void Plugin::setInfo(std::string s1, std::string s2)
{
    this->_libAPI->setInfo(s1, s2);
}

const std::string Plugin::getName()
{
    return this->_name;
}

void Plugin::runPlugin(CoreApi *_api)
{
    this->_libAPI->setConnection(_api);
    this->_libAPI->runPlugin();
}
