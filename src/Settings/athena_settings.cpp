#include "athena_settings.h"

#include <iostream>

AthenaSettings* AthenaSettings::instance = nullptr;
std::string AthenaSettings::settingsPath = "settings.txt";

//get full name of <name> file in app_data folder
std::string AthenaSettings::GetFullName(const std::string &name)
{
    std::string AppData_Path = SHARED_APP_DATA_PATH;

    boost::filesystem::path full_name = AppData_Path;
    full_name += "/AVSoft/AthenaAgent";

    if ( !boost::filesystem::is_directory(full_name) )
    {
        try {
            if (!boost::filesystem::create_directories(full_name)) {
                std::cerr << "Failed to create directory " << full_name << ".\n";
                return name;
            }
        }
        catch(const boost::filesystem::filesystem_error& e) {
            if(e.code() == boost::system::errc::permission_denied)
                std::cout << "Search permission is denied for one of the directories "
                          << "in the path prefix of " << full_name << "\n";
            else
                std::cout << "is_directory(" << full_name << ") failed with "
                          << e.code().message() << '\n';
            return name;
        }
    }

    full_name += "/" + name;
    return full_name.string();
}

json AthenaSettings::getProperty(const string& name) const
{
    if (_j.find(name) == _j.end())
        return json::object();
    return _j[name];
}
json AthenaSettings::getPluginSettingsProperty(const string& pluginName, const string& s_property) const
{
    try
    {
        if (_j.find("plugins") != _j.end())
        {
            for (auto it : _j["plugins"].items())
            {
                if (it.value()["name"].get<std::string>() == pluginName)
                    if (it.value().find("settings") != it.value().end() && it.value()["settings"].find(s_property) != it.value()["settings"].end())
                    {
                        return it.value()["settings"][s_property];
                    }
            }
        }

        return json::object();
    }
    catch (const json::exception& e)
    {
        cout << "parse error " << e.what() << endl;
        return json::object();
    }
}

set<string> AthenaSettings::getAvailablePluginsNames() const
{
    set<string> availableNames = set<string>();

    try
    {
        if (_j.find("plugins") == _j.end())
            return set<string>();

        for (auto plugin : _j["plugins"])
        {
            availableNames.insert(plugin["name"].dump());
        }

        return availableNames;

    }
    catch (const json::exception& e)
    {
        cout << "parse error " << e.what() << endl;
        return availableNames;
    }

}

void AthenaSettings::setProperty(const string& key, const json& value)
{
    _j[key] = value;
}

AthenaSettings::AthenaSettings(const string file_name)
{
    string full_name = GetFullName(file_name); //e.g C:/ProgramData/AVSoft/AthenaAgent/settings.txt
    std::ifstream binary_file(full_name.c_str(), std::ios_base::binary);
    if (binary_file)
    {
        binary_file >> _j;
    }
    else
    {
        _j["server"] = "athena-dev-04.avsw.ru";
        _j["port"] = "443";

        json _jObj;
        _jObj["name"] = "AgentPluginUI";
        _j["plugins"] = json::array({_jObj});

        _j["registration_target"] = "/agent_api/api/v1/register";
        _j["keepalive_target"] = "/agent_api/api/v1/keepalive";
        _j["confirmation_target"] = "/agent_api/api/v1/info";
        _j["log"] = "false";

        std::ofstream outfile (full_name);
        outfile << _j.dump(4);

        if(outfile.fail())
            cout << "Error: failed to write to " << full_name <<  "\n";
        outfile.close();
    }
    binary_file.close();
}

//json AthenaSettings::Json(void) const {  return this->_j; }
