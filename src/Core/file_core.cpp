#include "file_core.h"

FileCore::FileCore()
{
    setlocale(LC_ALL, "ru_Ru.UTF-8");

    this->files = new std::list<File*>;
    this->_is_run = false;
}

FileCore::~FileCore()
{
    this->stop_scan();

    for(auto &file : *this->files)
        delete file;

    delete this->files;
}

int FileCore::start_scan()
{
    if (core) {
        if(!this->_is_run)
        {
            this->_is_run = true;
            this->mainRun();
        }

        return 0;
    }
    else
        return -1;
}

int FileCore::stop_scan()
{
    if(this->_is_run)
    {
        this->_is_run = false;
    }
    // cv ?

    return 0;
}

int FileCore::addFile(std::string JSON)
{
    File *f = new File(JSON);
    return this->pushBackNotify(f);
}

int FileCore::addFile(File f)
{
    File *copy = new File(f);
    std::cout << "ADDING FILE " << f.hash << std::endl;
    return this->pushBackNotify(copy);
}

File* FileCore::searchFileForUniq(File *f)
{
    std::list<File*>::iterator it = this->files->begin();

    for (; it != this->files->end(); it++)
    {
        if ((*it)->hash == f->hash)
        {
            return (*it);
        }
    }
    return nullptr;
}

int FileCore::pushBackNotify(File *f)
{
    boost::mutex::scoped_lock lock(this->work_data_mutex);

    std::cout << "Notify!!! smthg happened!" << std::endl;

    if (searchFileForUniq(f) == nullptr)
    {
        this->files->push_back(f);
        cv.notify_one();
        return 0;
    }
    return -1;
}

void FileCore::threadFunc(FileCore* obj)
{
    while(obj->_is_run)
    {
        //obj->core->notify("HELOW", "ITS MI");

        boost::mutex::scoped_lock lock(obj->work_data_mutex);
        while (obj->files->size() == 0)
        {
            std::cout << "R na obed" << std::endl;
            obj->cv.wait(lock);
//            boost::this_thread::sleep(boost::posix_time::seconds(30));
            //continue;
        }

        File *workFile = obj->files->front();
        obj->files->pop_front();

        std::cout << "file state: " << workFile->status << std::endl;

        switch(workFile->status)
        {
        case -2:
        {
            obj->core->notify("file", workFile->toJSON());
            delete workFile;
            break;
        }
        case -1:
        {
            obj->deleteFromDB(workFile);
            workFile->status = -2;
            obj->files->push_back(workFile);
            break;
        }
        case 0: // только добавили файл
        {
            std::string ihih = workFile->hash;
            for(auto &e : *(obj->files))
            {
                if (e->hash == workFile->hash)
                {
                    workFile->status = -2; // с таким хэшем уже есть в списке
                    obj->files->push_back(workFile);
                    break;
                }

            }
            workFile->status = 1;
            obj->files->push_back(workFile);
            obj->core->notify("file", workFile->toJSON());
            break;
        }
        case 1: // добавили в список, проверяем дальше
        {
            int i;
            i = obj->checkHashInDB(workFile->hash).id;

            if (i == -1) // Файла в бд нет абсолютно
            {
                workFile->status = 3;
                obj->core->notify("file", workFile->toJSON());
            }
            else
            {
                obj->fillFileFromDB(workFile);
                // Файл найден в локальной бд & проверен на вебе (есть вердикт)
                if (workFile->status == 11)
                    ;

                // Файл в последний раз загружался, но инфа после этого по нему не обновилась в бд
                // Проверяем веб. То шо мы здесь оказались, уже показывает, что произошла ошибка при отправке.
                else if (workFile->status == 5)
                {
                    workFile->status = 2;
                }
                else if (workFile->status == 6
                         || workFile->status == 7
                         || workFile->status == 8
                         || workFile->status == 9
                         || workFile->status == 10)
                {
                    workFile->status = 7;
                }

            }
            obj->files->push_back(workFile);
            break;
        }
        case 2: //Файл есть в бд и последний раз виднелся на загрузке
        {

            if (workFile->webid) //Загрузка прошла успешно
                workFile->status = 7;
            else
            {
                workFile->status = 5;
            }

            obj->updateFileStatus(workFile);

            obj->files->push_back(workFile);

            break;
        }
        case 3: //В бд точно файла нет
        {
            std::string answer = "";
            //------------------------------------------------------------------
            // #5170 получение агентом информации о хеш-сумме из сервиса хешей
            // >> /hashinfo
/*
503 - сервис недоступен;
404 - хеш не найден в базе
200 - Хеш найден в базе, в контенте будет числовое значение вердикта
     (согласно описанию метода hash_fast, https://redmine.avsw.ru/projects/hash_service/wiki/Wiki )

0 - не определен
1 - безопасный
2 - подозрительный
3 - вредоносный
*/
            http::response<http::string_body> status_hash = obj->netCore->getFileStatusHash_h(workFile->hash);
            if (status_hash.result_int() == 200) // Хеш найден в базе, в контенте будет числовое значение вердикта
            {
                answer = status_hash.body();
                json a = json::parse(answer);
/*
(х) "HashID" (INT) - ID из таблицы
(х) "HashHash" (VARCHAR 64) - Хеш файла
(х) "HashVerdict" (INT 4) - Вердикт файла
(х) "HashType" (VARCHAR 15) - Тип хеша
"HashDescription" (TEXT) - Описание хеша
*/
                workFile->webid = a["HashID"];
                workFile->hash = a["HashHash"];
                int hashVerdict= a["HashVerdict"];
                switch(hashVerdict) {
                case 1: // безопасный / Benign
                    workFile->verdict = "Benign";
                    workFile->status = 11;
                    break;
                case 2: // подозрительный / Grayware
                    workFile->verdict = "Grayware";
                    workFile->status = 11;
                    break;
                case 3: // вредоносный / Malware
                    workFile->verdict = "Malware";
                    workFile->status = 11;
                    break;
                case 0: // не определен / Undefined
                default:
                    workFile->verdict = "Undefined";
                    break;
                }
            }

            if ( (status_hash.result_int() == 503 // сервис недоступен
                 || status_hash.result_int() == 404) // хеш не найден в базе
                || workFile->verdict == "Undefined" )
            {
                //------------------------------------------------------------------
                // Если не срослось - обращаемся по старому адресу
                // >> /fileinfo
                answer.clear();
                http::response<http::string_body> status = obj->netCore->getFileStatusHash_f(workFile->hash);
                if (status.result_int() == 404)
                {
                    workFile->status = 5;
                }
                else if(status.result_int() == 200)
                {
                    answer = status.body();
                    // Веб знает, парсим веб.
                    json a = json::parse(answer);
                    workFile->webid = a["Id"];
                    workFile->verdict = a["verdict"];
                    workFile->time = a["created_at"];
                    std::string status = a["status"];
                    if (status == "ExternalAnalysis" || status == "StaticAnalysis" || status == "Analysis")
                    {
                        workFile->status = 7;
                    }
                    else if (status == "StaticOnly" || status == "Verdict")
                    {
                        workFile->status = 11;
                    }
                    else if (status == "Pending" || status == "Removed" || status == "Undefined")
                    {
                        workFile->status = 5;
                    }
                }
                else
                {
                    workFile->status = -1;
                    workFile->webid = -1;
                    workFile->verdict = "Ошибка";
                    workFile->time = "01.01.1970 00:00:00";
                }
            }

            obj->addFileToDB(workFile);
            obj->files->push_back(workFile);
            obj->core->notify("file", workFile->toJSON());
            break;
        }
        case 4:
        {

            workFile->status = 11;
            obj->files->push_back(workFile);

            break;
        }
        case 5:
        {
            obj->core->notify("file", workFile->toJSON());
            http::response<http::string_body> a = obj->netCore->sendFile(workFile->path, workFile->hash);

            if(a.result_int() == 202) // The request has been accepted for processing, but the processing has not been completed.
            {
                json j = json::parse(a.body());

                workFile->webid = j["Id"];
                workFile->status = 6;
            }
            else
            {
                workFile->status = -1;
                workFile->webid = -1;
                workFile->verdict = "Ошибка";
                workFile->time = "01.01.1970 00:00:00";
            }
            obj->updateFileStatus(workFile);
            obj->files->push_back(workFile);

            obj->core->notify("file", workFile->toJSON());

            break;
        }
        case 6:
        {
            workFile->status = 7;
            obj->files->push_back(workFile);
            break;
        }
        case 7:
        {
            // Отправить ID
            boost::thread { [] (FileCore* obj, File* workFile)
                {
                    obj->core->notify("file", workFile->toJSON());
                    auto a = obj->netCore->getFileStatusID(workFile->webid);
                    bool parse_check = false;
                    int result_int = a.result_int();
                    if (result_int == 403/* || result_int == 400 ||  result_int == 500 || result_int == 503*/)
                    {
                        boost::this_thread::sleep(boost::posix_time::seconds(10));
                        workFile->status = 7;
                        obj->pushBackNotify(workFile);
                    }
                    else if (result_int == 404 || result_int == 401)
                    {
                        workFile->status = -1;
                        workFile->webid = -1;
                        workFile->verdict = "Ошибка";
                        workFile->time = "01.01.1970 00:00:00";
                        obj->core->notify("file", workFile->toJSON());
                        obj->pushBackNotify(workFile);
                    }
                    else if (result_int == 200)
                    {
                        json j;
                        try {
                            j = json::parse(a.body());
                            workFile->verdict = j["Verdict"];
                            if (j["State"] == "Complete")
                            {
                                workFile->status = 8;
                            }
                            else if (j["State"] == "Error")
                            {
                                workFile->status = -1;
                                workFile->webid = -1;
                                workFile->verdict = "Ошибка";
                                workFile->time = "01.01.1970 00:00:00";
                            }
                            else
                            {
                                boost::this_thread::sleep(boost::posix_time::seconds(15));
                                workFile->status = 7;
                            }
                            parse_check = true;
                            obj->pushBackNotify(workFile);
                        }
                        catch (...)
                        {
                            boost::this_thread::sleep(boost::posix_time::seconds(60));
                            workFile->status = 7;
                            obj->pushBackNotify(workFile);
                        }
                        if (!parse_check) {
                            printf("File check in web failed\n");
                        }
                    }

                }, obj, workFile}.detach();

            break;
        }
        case 8:
        {
            workFile->status = 11;
            obj->files->push_back(workFile);
            break;
        }
        case 9:
        {
            workFile->status = 7;
            obj->files->push_back(workFile);
            break;
        }
        case 10:
        {
            workFile->status = 7;
            obj->files->push_back(workFile);
            break;
        }
        case 11:
        {
            obj->updateFileStatus(workFile);
            obj->core->notify("file", workFile->toJSON());
            delete workFile;
            break;
        }
        }
    }
}

int FileCore::mainRun()
{
    this->t = new boost::thread(boost::bind(&FileCore::threadFunc, this));
    this->t->detach();
    return 0;
}

int FileCore::setDBManager(DBManager *dbm)
{
    this->DB = dbm->getDB("agent.db");
    return 0;
}

int FileCore::setPluginManager(PluginManager *_pm)
{
    this->core = _pm;
    return 0;
}

void FileCore::setNetCore(NetCore* _nc)
{
    this->netCore = _nc;

}

int FileCore::addFileToDB(File *f)
{
    // File должен быть уже заполнен:
    // path должен содержать путь к файлу
    // sha256 должен содержать хеш файла
    if (f->path.empty() || f->hash.empty())
        return -1;

    table_type files;

    const char* file_id  = nullptr;
    // Получить file_id
    // SELECT files.id FROM files WHERE files.sha256 = f.hash
    std::string query;
    query = "SELECT files.id FROM files WHERE files.sha256 = \"" + f->hash + "\"";
    //printf("Query: '%s'\n", query.c_str());
    int rec = DB->execSQLQuery_raw(query, callbackDB, &files);

    if (rec == 0 && files.size() > 0)
    {
        file_id = files.at(0).at(0).data();
        return -1;
    }
    else
    {
        //printf("File id not found\n");

        // Если нету, то:
        //    Добавить файл в таблицу files
        //    INSERT INTO files (sha256, status, verdict)
        //      VALUES (f.hash, 0, 0)
        // Повторить get file_id
        query = "INSERT INTO files (sha256, status, verdict, time) "
                " VALUES (\"" + f->hash + "\", 0, (SELECT id FROM verdict WHERE verdict = \"" + f->verdict + "\"), \"" + f->time + "\")";
        //printf("Query: '%s'\n", query.c_str());
        DB->execSQLQuery_raw(query.c_str(), nullptr, nullptr);

        query = "SELECT files.id FROM files WHERE files.sha256 = \"" + f->hash + "\"";
        files.clear();
        //printf("Query: '%s'\n", query.c_str());
        DB->execSQLQuery_raw(query.c_str(), callbackDB, &files);

        if (files.size() > 0)
        {
            file_id = files.at(0).at(0).data();
        }
    }
    if (!file_id)
        return -1;
    // Если есть:
    //printf("FILE ID: %s\n", file_id);

    // Добавить путь в таблицу filepath
    //   INSERT INTO filepath (path, file_id)
    //     VALUES (f.path, file_id)
    query = "INSERT INTO filepath (path, file_id) VALUES (\"" + f->path + "\", \"" + file_id + "\")";

    query = "SELECT id FROM filepath WHERE path = \"" + f->path + "\"";

    rec = DB->execSQLQuery_raw(query.c_str(), callbackDB, &files);
    const char * fp_id = nullptr;

    if (rec == 0 && files.size() > 0)
    {
        fp_id = files.at(0).at(0).data();

        query = "INSERT INTO filepath (path, file_id) VALUES (\"" + f->path + "\", \"" + fp_id + "\")";
        DB->execSQLQuery_raw(query.c_str(), nullptr, nullptr);
    }

    string webid = to_string(f->webid);
    query = "INSERT INTO web_id (web_id, file_id) VALUES (\"" + webid + "\", \"" + file_id + "\")";
    DB->execSQLQuery_raw(query.c_str(), nullptr, nullptr);

    f->id = std::atoi(file_id);
    printf("ADDED FILE TO DB!!!!: %s\n", f->toJSON().c_str());
    return 0;
}

int FileCore::updateFileStatus(File *f)
{
    if (f->path.empty() || f->hash.empty())
        return -1;

    table_type files;

    const char* file_id  = nullptr;
    // Получить file_id

    std::string query;
    query = "SELECT files.id FROM files WHERE files.sha256 = \"" + f->hash + "\"";
    printf("Query: '%s'\n", query.c_str());

    int rec = DB->execSQLQuery_raw(query, callbackDB, &files);

    if (rec == 0 && files.size() > 0)
    {
        file_id = files.at(0).at(0).data();
    }
    if (!file_id)
        return -1;

    // Если есть:
    // изменить статус строки в таблице под id file_id

    // Добавить путь в таблицу filepath
    //   INSERT INTO filepath (path, file_id)
    //     VALUES (f.path, file_id)
    query = "UPDATE files SET status = " + std::to_string(f->status) + " WHERE files.id = " + file_id;
    DB->execSQLQuery_raw(query, nullptr, nullptr);

    string webid = to_string(f->webid);
    query = "UPDATE web_id SET web_id = " + std::to_string(f->webid) + " WHERE web_id.file_id = " + file_id;
    DB->execSQLQuery_raw(query, nullptr, nullptr);

    query = "UPDATE files SET verdict = (SELECT id FROM verdict WHERE verdict = \"" + f->verdict + "\")" + " WHERE id = " + file_id;
    DB->execSQLQuery_raw(query, nullptr, nullptr);

    printf("UPDATED FILE TO DB!!!!: %s\n", f->toJSON().c_str());
    return 0;
}

File FileCore::checkHashInDB(string _hash)
{
    std::string query;

    table_type files;
    query = "SELECT files.id, files.sha256, verdict.verdict, files.status, web_id.web_id, files.time\
            FROM files\
            JOIN verdict ON verdict.id = files.verdict\
            JOIN web_id ON files.id = web_id.file_id\
            WHERE files.sha256 = \"" + _hash + "\"";
    //printf("Query: '%s'\n", query.c_str());
    int rec = DB->execSQLQuery_raw(query.c_str(), callbackDB, &files);
    File f;

    f.id = -1; //"-1";

    if (rec == 0 && files.size() > 0)
    {
        f.id = std::atoi(files.at(0).at(0).data());
        f.hash = files.at(0).at(1).data();
        f.verdict = files.at(0).at(2).data();
        f.status = std::atoi(files.at(0).at(3).data());
        //cout << "fillin: " << f.id << " " << f.hash << " " << f.verdict << " " << f.status << endl;
    }
    return f;
}

string FileCore::getSHA256(string str)
{
    std::ifstream f(str.c_str(), std::ios::binary);

    std::vector<unsigned char> s(picosha2::k_digest_size);
    if (f.is_open())
    {
        printf("%s",str.c_str());
        picosha2::hash256(f, s.begin(), s.end());
    }
    else
    {
        cout << "getSHA256 Error\n";
        return "-1";
    }
    std::string hex_str = picosha2::bytes_to_hex_string(s.begin(), s.end());
    return hex_str;
}

int FileCore::fillFileFromDB(File *f)
{
    std::string query;

    table_type files;

    /*FROM files\
             JOIN verdict ON verdict.id = files.id\
             JOIN web_id ON web_id.id = files.id \*/

    query = "SELECT files.id, files.sha256, verdict.verdict, files.status, web_id.web_id, files.time \
            FROM files\
            JOIN verdict ON verdict.id = files.verdict\
            JOIN web_id ON files.id = web_id.file_id\
            WHERE files.sha256 = \"" + f->hash + "\"";

    int rec = DB->execSQLQuery_raw(query.c_str(), callbackDB, &files);

    f->id = -1; //"-1";

    if (rec == 0 && files.size() > 0)
    {
        f->id = std::atoi(files.at(0).at(0).data());
        f->hash = files.at(0).at(1).data();
        f->verdict = files.at(0).at(2).data();
        f->status = std::atoi(files.at(0).at(3).data());
        f->webid = std::atoi(files.at(0).at(4).data());
        f->time = files.at(0).at(5).data();
        cout << "fillin: " << f->id << " " << f->hash << " " << f->verdict << " " << f->status << " " << f->webid << " " << f->time << endl;
    }
    return 0;
}

int FileCore::fillSHA256(File* _f)
{
    std::ifstream f(_f->path.c_str(), std::ios::binary);
    std::vector<unsigned char> s(picosha2::k_digest_size);

    picosha2::hash256(f, s.begin(), s.end());

    _f->hash = picosha2::bytes_to_hex_string(s.begin(), s.end());

    return 0;
}

int FileCore::deleteFromDB(File* _f)
{
    if (this->DB)
    {
        std::string delQueryFiles = "DELETE FROM files WHERE files.sha256=\"" + _f->hash + "\";";
        std::string delQueryWeb = "DELETE FROM web_id WHERE web_id.id=\"" + to_string(_f->id) + "\";";
        std::string delQueryFilepath = "DELETE FROM filepath WHERE filepath.id=\"" + to_string(_f->id) + "\"";

        std::string resultQuery = delQueryFiles + delQueryWeb + delQueryFilepath;
        int rec = DB->execSQLQuery_raw(resultQuery.c_str(), NULL, NULL);
        if (rec == 0)
        {
            std::cout << "Deleted " << _f->path << " from DB" << endl;
        }
    }
    return 0;
}

int FileCore::prepareDB()
{
    if (this->DB)
    {
        DB->execSQLQuery("CREATE TABLE IF NOT EXISTS \"filepath\" ( `id` INTEGER, `path` TEXT, `file_id` INTEGER, PRIMARY KEY(`id`) )");
        DB->execSQLQuery("CREATE TABLE IF NOT EXISTS \"files\" ( `id` INTEGER, `sha256` TEXT unique, `status` INTEGER DEFAULT 0, `verdict` INTEGER DEFAULT 3, `time` TEXT, PRIMARY KEY(`id`) )");
        DB->execSQLQuery("CREATE TABLE IF NOT EXISTS \"status\" ( id INTEGER PRIMARY KEY, status TEXT )");
        DB->execSQLQuery("CREATE TABLE IF NOT EXISTS \"verdict\" ( id INTEGER PRIMARY KEY, verdict TEXT)");
        DB->execSQLQuery("CREATE TABLE IF NOT EXISTS \"web_id\" (`file_id` INTEGER, `web_id` INTEGER, `id`	INTEGER, PRIMARY KEY(`id`) )");
        DB->execSQLQuery("INSERT INTO \"status\" (id, status) VALUES (-1, 'Error')");
        DB->execSQLQuery("INSERT INTO \"status\" (id, status) VALUES (0, 'Undefined')");
        DB->execSQLQuery("INSERT INTO \"status\" (id, status) VALUES (1, 'SearchLocalDB')");
        DB->execSQLQuery("INSERT INTO \"status\" (id, status) VALUES (2, 'FindLocalDB')");
        DB->execSQLQuery("INSERT INTO \"status\" (id, status) VALUES (3, 'SearchHash')");
        DB->execSQLQuery("INSERT INTO \"status\" (id, status) VALUES (4, 'FindHash')");
        DB->execSQLQuery("INSERT INTO \"status\" (id, status) VALUES (5, 'Uploading')");
        DB->execSQLQuery("INSERT INTO \"status\" (id, status) VALUES (6, 'Uploaded')");
        DB->execSQLQuery("INSERT INTO \"status\" (id, status) VALUES (7, 'CheckSandBox')");
        DB->execSQLQuery("INSERT INTO \"status\" (id, status) VALUES (8, 'SandBox')");
        DB->execSQLQuery("INSERT INTO \"status\" (id, status) VALUES (9, 'SendID')");
        DB->execSQLQuery("INSERT INTO \"status\" (id, status) VALUES (10, 'CompleteID')");
        DB->execSQLQuery("INSERT INTO \"status\" (id, status) VALUES (11, 'Complete')");
        DB->execSQLQuery("INSERT INTO \"verdict\" (id, verdict) VALUES (0, 'Benign')");
        DB->execSQLQuery("INSERT INTO \"verdict\" (id, verdict) VALUES (1, 'Grayware')");
        DB->execSQLQuery("INSERT INTO \"verdict\" (id, verdict) VALUES (2, 'Malware')");
        DB->execSQLQuery("INSERT INTO \"verdict\" (id, verdict) VALUES (3, 'Undefined')");
    }
    return 0;
}

