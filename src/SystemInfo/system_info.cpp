#include "system_info.h"

#ifdef _WIN32
#include <windows.h>
#include <comdef.h>
#include <wbemidl.h>
// #pragma comment(lib, "wbemuuid.lib")
#endif

std::string SystemInfo::uuid = get_uuid();
std::string SystemInfo::machine_name = get_machine_name();

#ifdef _WIN32
namespace _com_util
{
    //------------------------//
    // Convert char * to BSTR //
    //------------------------//
    wchar_t* ConvertStringToBSTR(const char* pSrc)
    {
        if(!pSrc) return NULL;

        DWORD cwch;

        BSTR wsOut(NULL);

        if(cwch = ::MultiByteToWideChar(CP_ACP, 0, pSrc,
             -1, NULL, 0))//get size minus NULL terminator
        {
                    cwch--;
                wsOut = ::SysAllocStringLen(NULL, cwch);

            if(wsOut)
            {
                if(!::MultiByteToWideChar(CP_ACP,
                         0, pSrc, -1, wsOut, cwch))
                {
                    if(ERROR_INSUFFICIENT_BUFFER == ::GetLastError())
                        return wsOut;
                    ::SysFreeString(wsOut);//must clean up
                    wsOut = NULL;
                }
            }

        };

        return wsOut;
    };

    //------------------------//
    // Convert BSTR to char * //
    //------------------------//
    char* ConvertBSTRToString(wchar_t* pSrc)
    {
        if(!pSrc) return NULL;

        //convert even embeded NULL
        DWORD cb,cwch = SysStringLen(pSrc);

        char *szOut = NULL;

        if(cb = WideCharToMultiByte(CP_ACP, 0,
                   pSrc, cwch + 1, NULL, 0, 0, 0))
        {
            szOut = new char[cb];
            if(szOut)
            {
                szOut[cb - 1]  = '\0';

                if(!WideCharToMultiByte(CP_ACP, 0,
                    pSrc, cwch + 1, szOut, cb, 0, 0))
                {
                    delete []szOut;//clean up if failed;
                    szOut = NULL;
                }
            }
        }

        return szOut;
    };
}
#endif

std::string SystemInfo::get_machine_name(void)
{

#ifdef __linux__
    std::string result = "Test machine";
    std::ifstream file("/proc/sys/kernel/hostname");

    if(file.is_open()){
        std::getline(file, result);

        file.close();
    }
    return result;
#endif

#ifdef _WIN32
        std::string result;
        DWORD size = 0;
        GetComputerNameA(0, &size);
        result.reserve(size);
        char buf[255];
        GetComputerNameA(buf, &size);
        result = buf;
        return result;
#endif
}

std::string SystemInfo::get_uuid(void)
{
    std::string result = "00dead00-0000-0000-0000-00deadbeef00";

#ifdef __linux__
    std::ifstream file("/sys/devices/virtual/dmi/id/product_uuid");

    if(file.is_open()){
        std::getline(file, result);

        file.close();
    }
#endif

#ifdef _WIN32

    HRESULT hres;


    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres))
    {
        return "";                  // Program has failed.
    }

    hres = CoInitializeSecurity(
        NULL,
        -1,                          // COM authentication
        NULL,                        // Authentication services
        NULL,                        // Reserved
        RPC_C_AUTHN_LEVEL_DEFAULT,   // Default authentication
        RPC_C_IMP_LEVEL_IMPERSONATE, // Default Impersonation
        NULL,                        // Authentication info
        EOAC_NONE,                   // Additional capabilities
        NULL                         // Reserved
    );


    if (FAILED(hres))
    {
        CoUninitialize();
    }

    IWbemLocator* pLoc = NULL;

    hres = CoCreateInstance(
        CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&pLoc);

    if (FAILED(hres))
    {
        CoUninitialize();
    }

    IWbemServices* pSvc = NULL;

    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"), // Object path of WMI namespace
        NULL,                    // User name. NULL = current user
        NULL,                    // User password. NULL = current
        0,                       // Locale. NULL indicates current
        NULL,                    // Security flags.
        0,                       // Authority (for example, Kerberos)
        0,                       // Context object
        &pSvc                    // pointer to IWbemServices proxy
    );

    if (FAILED(hres))
    {
        pLoc->Release();
        CoUninitialize();
    }

    hres = CoSetProxyBlanket(
        pSvc,                        // Indicates the proxy to set
        RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
        RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
        NULL,                        // Server principal name
        RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx
        RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
        NULL,                        // client identity
        EOAC_NONE                    // proxy capabilities
    );

    if (FAILED(hres))
    {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
    }

    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t("SELECT * FROM Win32_ComputerSystemProduct"),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator);

    if (FAILED(hres))
    {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();              // Program has failed.
    }

    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;

    while (pEnumerator)
    {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1,
            &pclsObj, &uReturn);

        if (0 == uReturn)
        {
            break;
        }

        VARIANT vtProp;

        hr = pclsObj->Get(L"UUID", 0, &vtProp, 0, 0);
        result = (const char*)_bstr_t(V_BSTR(&vtProp));
        VariantClear(&vtProp);

        pclsObj->Release();
    }

    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();
#endif

    return result;
}
