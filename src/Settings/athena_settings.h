#ifndef ATHENA_SETTINGS_H
#define ATHENA_SETTINGS_H

#include "../Deps/json.hpp"

#include <fstream>
#include <stdio.h>
#include <string>
#include <set>
#include <boost/filesystem.hpp>

using json = nlohmann::json;
using namespace std;

class AthenaSettings
{
    json _j;

public:

    static void setSettingsPath(const string& path)
    {
        settingsPath = path;
    }

    static AthenaSettings* getSettings()
    {
        if (!instance)
        {
            instance = new AthenaSettings(settingsPath);
        }
        return instance;
    }
    
   // json Json(void) const;

    json getProperty(const string& name) const;
    json getPluginSettingsProperty(const string& pluginName, const string& s_property) const;
    set<string> getAvailablePluginsNames() const;

    void setProperty(const string& key, const json& value);

private:
    std::string GetFullName(const string& name);

    AthenaSettings(const string);

    static AthenaSettings* instance;
    static std::string settingsPath;
};

#endif
