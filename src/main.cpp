#include <stdio.h>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/exception/all.hpp>
#include <boost/chrono.hpp>
#include <version.h>

#include "Core/agent_core.h"

#include "NetApi/athena_request.h"

using namespace std;
using namespace boost::filesystem;

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/file.h>
#include <errno.h>
#endif

int main(int argc, char* argv[])
{
#ifdef _WIN32
    // ensure only one running instance
    HANDLE hMutexHandle = CreateMutex(NULL, TRUE, "AthenaAgentMutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        printf("Error: Another copy of AthenaAgent is already runnig!\n"); // another instance is running
        return 1;
    }
#else
    int pid_file = open("/tmp/.athenaAgent.pid\n", O_CREAT | O_RDWR, 0666);
    int rc = flock(pid_file, LOCK_EX | LOCK_NB);
    if(rc) {
        if(EWOULDBLOCK == errno) {
            printf("Error: Another copy of AthenaAgent is already runnig!\n"); // another instance is running
            return -1;
        }
    }
#endif

    setlocale(LC_ALL, "rus");

#ifdef _WIN32

    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    HWND h = GetConsoleWindow();
    ShowWindow(h, 0);

    if (argc > 1)
    {
        for(int i = 0; i < argc; i++)
        {
            if (!strcmp(argv[i], "--debug"))
            {
                HWND h = GetConsoleWindow();
                ShowWindow(h, 1);
                std::cout << " show windows" << std::endl;
                continue;
            }
            if (!strcmp(argv[i], "--path"))
            {
                std::string path = argv[++i];
                std::cout << "Set agent work path to: " << path << std::endl;

                if ( !SetCurrentDirectoryA(path.c_str()) )
                {
                    std::cout << "SetCurrentDirectoryA failed!" << std::endl;
                }
            }
        }
    }
    else
    {
        HWND h = GetConsoleWindow();
        ShowWindow(h, 0);
        FreeConsole();
        std::cout << " hide windows" << std::endl;
    }
    //EnableWindow(h, false);
#endif


    printf("Athena Agent version %d.%d.%d!\n", AthenaAgent_VERSION_MAJOR, AthenaAgent_VERSION_MINOR, AthenaAgent_VERSION_PATCH);

    AgentCore* agentCore = new AgentCore();
    agentCore->init();

    agentCore->start()->join();

    delete agentCore;

#ifdef _WIN32
    ReleaseMutex(hMutexHandle);
    CloseHandle(hMutexHandle);
#endif

    return 0;
}
