#ifndef UUID_H
#define UUID_H

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/lexical_cast.hpp>
#include <string>
#include <fstream>

class SystemInfo
{
private:
   static std::string get_machine_name(void) ;
   static std::string get_uuid(void) ;

public:
    static std::string uuid;
    static std::string machine_name;
};

#ifdef _WIN32
/*
https://docs.microsoft.com/en-us/previous-versions/windows/desktop/automat/bstr

typedef WCHAR OLECHAR;
typedef OLECHAR* BSTR;
typedef BSTR* LPBSTR;

*/
namespace _com_util
{
    wchar_t* ConvertStringToBSTR(const char* pSrc);
    char* ConvertBSTRToString(wchar_t* pSrc);
}
#endif

#endif
