#ifndef FILE_H
#define FILE_H
#include <string>
#include <iostream>
#include <sstream>

#include "../Deps/json.hpp"

using json = nlohmann::json;
class File
{
public:
    File() {
        id = 0;
        name = "test_file";
        path = "/test/test_file";
        hash = "f2ca1bb6c7e907d06dafe4687e579fce76b37e4e93b7605022da52e6ccc26fd2";
        status = 0;
        verdict = "Test verdict";
        time = "0";
        webid = 0;
    }
    File(std::string JSON) {
        fillFromJson(JSON);

        printf("\nPATH: %s\n", path.c_str());
    }

    int id;
    std::string name;
    std::string path; // путь+имя_файла
    std::string hash;
    int status;
    std::string verdict;
    std::string time;
    int webid;

    std::string toJSON() {
        std::stringstream ss;
        ss << "{" <<
            "\"id\":" << id << "," <<
            "\"name\":" << "\"" << name << "\"," <<
            "\"path\":" << "\"" << path << "\"," <<
            "\"hash\":" << "\"" << hash << "\"," <<
            "\"status\":" << status << ","
            "\"verdict\":" << "\"" << verdict << "\"," <<
            "\"time\":" << "\"" << time << "\"," <<
            "\"webid\":" << webid <<
            "}";
        return ss.str();
    }

    void fillFromJson(std::string JSON) {
        json j;
        try {
            //bool is_inBrack = false;
            //std::remove_if(JSON.begin(), JSON.end(), [&is_inBrack](char &ch) -> bool
            //    {
            //        if (ch == '\"')
            //        {
            //            is_inBrack = !is_inBrack;
            //        }
            //        else if (ch == ' ')
            //        {
            //            if (is_inBrack)
            //                return false;
            //            else
            //                return true;
            //        }
            //        return false;
            //    });
            //is_inBrack = false;

            //auto getContent = [&is_inBrack,&JSON](size_t offset) -> std::string
            //{
            //    std::string result;
            //    size_t cO = JSON.find(':', offset);
            //    if (JSON[cO + 1] == '\"')
            //    {
            //        size_t i = cO + 2;
            //        while (JSON[i] != '\"')
            //        {
            //            result.push_back(JSON[i]);
            //            i++;
            //        }
            //    }
            //    else {
            //        size_t i = cO + 1;
            //        while (JSON[i] != ',' && JSON[i] != '}')
            //        {
            //            result.push_back(JSON[i]);
            //            i++;
            //        }
            //    }
            //    return result;
            //};
            //name = getContent(JSON.find("\"name\""));
            //path = getContent(JSON.find("\"path\""));
            //hash = getContent(JSON.find("\"hash\""));
            //verdict = getContent(JSON.find("\"verdict\""));
            //time = getContent(JSON.find("\"time\""));
            //id = ::atoi(getContent(JSON.find("\"id\"")).c_str());
            //status = ::atoi(getContent(JSON.find("\"status\"")).c_str());
            //webid = ::atoi(getContent(JSON.find("\"webid\"")).c_str());
            //printf("%s\n", path.c_str());
            j = json::parse(JSON);
            id = j["id"];
            name = j["name"];
            path = j["path"];
            hash = j["hash"];
            status = j["status"];
            verdict = j["verdict"];
            time = j["time"];
            webid = j["webid"];
        } catch (const nlohmann::json::parse_error& e) {
            printf("Parse error, %s\n%s", e.what(), JSON.c_str());
        }
        catch (const nlohmann::json::type_error& e) {
            printf("Type error, %s\n", e.what());
        }
        catch (const nlohmann::json::other_error& e) {
            printf("Other error, %s\n", e.what());
        }
    }
};

#endif // FILE_H
