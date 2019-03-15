// stdafx.h: включаемый файл для стандартных системных включаемых файлов
// или включаемых файлов для конкретного проекта, которые часто используются, но
// нечасто изменяются
//Этот файл относится к проекту SignProjectTool

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Исключите редко используемые компоненты из заголовков Windows
// Файлы заголовков Windows
#include <windows.h>
#include <ShObjIdl.h>
// Файлы заголовков среды выполнения C
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <string>
#include <vector>
//Файлы заголовков проектных DLL
#include "../WWS/WWS.h"
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
typedef std::vector<TCHAR> TCHARVECTOR;
typedef std::vector<UINT> UINTVECTOR;
#ifdef _UNICODE 
typedef std::wstring TSTRING;
typedef std::vector<TSTRING> TSTRINGARRAY;
#define to_tstring(x) std::to_wstring(x)
#else
typedef std::string TSTRING;
typedef std::vector<TSTRING> TSTRINGARRAY;
#define to_tstring(x) std::to_string(x)
#endif

// установите здесь ссылки на дополнительные заголовки, требующиеся для программы
