#include "plugin_manager.h"

PluginManager::PluginManager()
{
    subscribers = new std::list<Plugin*>;
    settings = AthenaSettings::getSettings();
}

PluginManager::~PluginManager()
{
    delete subscribers;

    map<string, Plugin *>::iterator it;

    for (it = this->plugins.begin(); it != this->plugins.end(); ++it)
        delete it->second;
}

Plugin* PluginManager::getPlugin(const string &key)
{
    return (this->plugins.find(key) != this->plugins.end()) ?
                this->plugins[key] : nullptr;
}

void PluginManager::notify(std::string from, std::string what)
{
    std::list<Plugin*>::iterator it;
    for (it = (*this->subscribers).begin(); it != (*this->subscribers).end(); ++it) {
        (*it)->setInfo(from, what);
    }
}

int PluginManager::subscribePlugin(std::string pluginName)
{
    Plugin* s = this->getPlugin(pluginName);
    int ret = 0;
    if (this->subscribers && s)
    {
        this->subscribers->push_back(s);
        ret = 0;
    }
    else
    {
        ret = -1;
    }
    return ret;
}

int PluginManager::_parsePluginsDir(path pluginsDir)
{
    std::cout << "======= From PluginManager::_parsePluginsDir(path pluginsDir) ==========" << std::endl;
    directory_iterator end_itr;
    directory_iterator itr;

    try
    {
        set<string> availableNames = settings->getAvailablePluginsNames();

        for (itr = directory_iterator(pluginsDir); itr != end_itr; ++itr)
        {
            string filename = itr->path().filename().string();
            filename = filename.substr(0, filename.find_last_of('.'));

          //  cout << (extension(itr->path()) == ".so" && availableNames.count("\"" + filename.substr(3) + "\""))  << endl;

            if (is_regular_file(itr->path())
                && (
                    (extension(itr->path()) == ".dll" && availableNames.count("\"" + filename + "\"")) //если .dll, то ищем просто совпадение
                    || (extension(itr->path()) == ".so" && availableNames.count("\"" + filename.substr(3) + "\"")) //если .so, то отбрасываем "lib"
                    )
                )

            {
                Plugin *p = new Plugin();

                printf("%s\n", itr->path().string().c_str());
                if (!p->init(itr->path().string()))
                {
                    this->plugins[p->getName()] = p;
                    pluginsStatus.insert( std::pair<std::string, short>(p->getName(), 1) );
                }
                else
                {
                    delete p;
                }
            }
        }
    }
    catch (boost::filesystem::filesystem_error& ex)
    {
        printf("Plugin parse error. Exit\n");
        (void)(ex);
        return -1;
    }
    catch (json::exception &ex)
    {
        printf("%s %s", "Configurations file error while detecting plugins", ex.what());
        return-1;
    }
    return 0;
}

map<string, Plugin*>& PluginManager::getPlugins()
{
    std::cout << "From PluginManager::getPlugins()" << std::endl;
    return plugins;
}


map<string, short>& PluginManager::getPluginsInfo()
{
    return pluginsStatus;
}
short PluginManager::getPluginInfo(string pluginName)
{
    return pluginsStatus[pluginName];
}
void PluginManager::setPluginInfo(string pluginName, short status)
{
    pluginsStatus[pluginName] = status;
}


int PluginManager::loadPlugins()
{
    std::cout << "=*=*=*=*=* From PluginManager::loadPlugins() !!!!" << std::endl;
    path p (".");
    p = p.native();
    std::cout << "\t p = " << p << std::endl;

    int res = -1;
    directory_iterator end_itr;
    directory_iterator itr;

    // Сначала найдём нужную папку с плагинами (потом можно будет составить список имён папок)
    try
    {
        std::cout << "=*=*=*=*=* In Try block !!! " << std::endl;
        for (itr = directory_iterator(p); itr != end_itr; ++itr)
        {
            std::cout << "\t itr = " << *itr << std::endl;
            if (is_directory(itr->path()))
            {
                string dirName = itr->path().filename().string();
                std::cout << "\t\t dirName = " << dirName << std::endl;
                if (!dirName.compare("plugins"))
                {
                    _parsePluginsDir(itr->path());
                }
            }
        }
    } 

    catch (boost::filesystem::filesystem_error& ex)
    {
        std::cout << "=*=*=*=*=* In Catch block !!! " << std::endl;
        printf("Plugin parse error. Exit\n");
        (void)(ex);
        res = -2;
        return res;
    }

    std::cout << "=*=*=*=*=* End of PluginManager::loadPlugins() !!!!" << std::endl;

    return res;
}
