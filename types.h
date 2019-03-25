#pragma once
#include <string>
//#include <string.h>
#include <vector>
#include <tchar.h>
typedef std::wstring WSTRING;
typedef std::vector<WSTRING> WSTRINGARRAY;
typedef std::string ASTRING, STRING;
typedef std::vector<ASTRING> ASTRINGARRAY;
typedef std::vector<TCHAR> TCHARVECTOR;
typedef std::vector<UINT> UINTVECTOR;
typedef std::vector<CHAR> CHARVECTOR;
typedef std::vector<WCHAR> WCHARVECTOR;
#ifdef _UNICODE 
typedef std::wstring TSTRING;
typedef std::vector<TSTRING> TSTRINGARRAY;
#define to_tstring(x) std::to_wstring(x)
#else
typedef std::string TSTRING;
typedef std::vector<TSTRING> TSTRINGARRAY;
#define to_tstring(x) std::to_string(x)
#endif