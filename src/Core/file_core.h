#ifndef FILECORE_H
#define FILECORE_H
#include "../Plugin/plugin_manager.h"
#include <list>
#include <boost/thread.hpp>
#include "../DB/dbmanager.h"
#include "../Deps/picosha2.h"
#include "../NetApi/net_core.h"


typedef vector<vector<string> > table_type;
static int callbackDB(void *ptr, int argc, char* argv[], char* cols[])
{
    if (ptr == NULL)
        return 0;

    table_type* table = static_cast<table_type*>(ptr);
    vector<string> row;
    for (int i = 0; i < argc; i++)
    {
        row.push_back(argv[i] ? argv[i] : "(NULL)");
        //printf("argv[%d]: %s\n", i, argv[i]);
    }
    table->push_back(row);
    return 0;
}



class FileCore
{
public:
    FileCore();
    ~FileCore();
    int start_scan();
    int stop_scan();

    int addFile(File f);
    int addFile(std::string JSON);

    int mainRun();
    int setDBManager(DBManager* _dbm);
    int setPluginManager(PluginManager* _pm);
    void setNetCore(NetCore* _nc);

    int addFileToDB(File *f);

    // old style
    File checkHashInDB(std::string _hash);
    std::string getSHA256(std::string str);

    // new
    int fillFileFromDB(File *_f);
    int fillSHA256(File *_f);
    int deleteFromDB(File* _f);

    int prepareDB();

    static void threadFunc(FileCore *obj);
    int NotifyWorker(File *f);
    int pushBackNotify(File *f);
    File *searchFileForUniq(File *f);

    int updateFileStatus(File *f);
private:
    std::list<File*>* files;
    PluginManager* core;
    NetCore* netCore;
    bool _is_run;

    boost::thread *t;
    boost::mutex work_data_mutex;
    boost::condition_variable cv;

    AgentDB* DB;
};

#endif // FILECORE_H
