#pragma once
#include <string>
//#include <string.h>
#include <vector>
//Unicode
typedef std::wstring WSTRING;
typedef std::vector<WSTRING> WSTRINGARRAY;
typedef std::vector<UINT> UINTVECTOR;
typedef std::vector<WCHAR> WCHARVECTOR;
//ASCII
typedef std::string ASTRING, STRING;
typedef std::vector<ASTRING> ASTRINGARRAY, STRINGARRAY;
typedef std::vector<CHAR> ACHARVECTOR, CHARVECTOR;