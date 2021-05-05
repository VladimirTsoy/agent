#include "agentdb.h"

#include <iostream>

/*!
 * \brief Подключение к базе
 * \param Путь до базы
 * \return Возвращает индекс в векторе в случае успеха, иначе -1
 */
AgentDB::AgentDB(const std::string &path)
{
    std::string fullpath = GetUserAppsDir(path);

    // Сохранить структуру подключения в переменной класса
    int rc = sqlite3_open(fullpath.c_str(), &(this->connection));
    if (rc != SQLITE_OK) {
        printf("Error connecting to DB '%s'\n", fullpath.c_str());
        this->connection = nullptr;
        return;
    }
}

AgentDB::~AgentDB()
{
    sqlite3_close(connection);
}

int AgentDB::execSQLQuery(const std::string &query)
{
    char* messageError;
    int rc;

    rc = sqlite3_exec(connection, query.c_str(), nullptr, nullptr, &messageError);

    if (rc != SQLITE_OK)
    {
        printf("Error executing query. %s\n", messageError);
        sqlite3_free(messageError);
        return -1;
    }
    else printf("Query '%s' was succesfully executed!\n", query.c_str());

    return 0;
}

int AgentDB::execSQLQuery_raw(const std::string &query, int(*callback)(void*, int ,char**, char**), void* StructToFill)
{
    char* messageError = nullptr;
    int rc;

    rc = sqlite3_exec(connection, query.c_str(), callback, StructToFill, &messageError);

    if (rc != SQLITE_OK)
    {
        printf("Error executing query... %s", messageError ? messageError : "");
        sqlite3_free(messageError);
    }

    return rc;
}

/*!
 * \brief AgentDB::GetUserAppsDir - получить путь (директория + имя файла)
 *
 * Если получится - создаётся каталог в Linux/Windows
 * если нет - просто дадим обратно имя файла как есть
 * (файл тогда создастся в рабочем каталоге)
 *
 * \param path - имя файла
 * \return путь + имя файла
 */
std::string AgentDB::GetUserAppsDir(const std::string &path)
{
    std::string AppData_Path = SHARED_APP_DATA_PATH;

    std::cout << "AppData: " << AppData_Path << std::endl;

    boost::filesystem::path full_path = AppData_Path;
    full_path += "/AVSoft/AthenaAgent";

    if ( !boost::filesystem::is_directory(full_path) )
    {
        try {
            if (!boost::filesystem::create_directories(full_path)) {
                std::cerr << "Failed to create directory " << full_path << ".\n";
                return path;
            }
        }
        catch(const boost::filesystem::filesystem_error& e) {
            if(e.code() == boost::system::errc::permission_denied)
                std::cout << "Search permission is denied for one of the directories "
                          << "in the path prefix of " << full_path << "\n";
            else
                std::cout << "is_directory(" << full_path << ") failed with "
                          << e.code().message() << '\n';
            return path;
        }
    }

    full_path += "/" + path;
    std::cout << "Agent  : " << full_path.string() << std::endl;
    return full_path.string();
}
