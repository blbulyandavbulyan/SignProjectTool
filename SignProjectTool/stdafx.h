// stdafx.h: включаемый файл для стандартных системных включаемых файлов
// или включаемых файлов для конкретного проекта, которые часто используются, но
// нечасто изменяются
//

#pragma once

#include "targetver.h"

//#define WIN32_LEAN_AND_MEAN             // Исключите редко используемые компоненты из заголовков Windows
// Файлы заголовков Windows
#include <windows.h>

// Файлы заголовков среды выполнения C
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <string>
#include <strsafe.h>
#include <vector>
#include <ShObjIdl.h>
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
typedef std::vector<TCHAR> TCHARVECTOR;
typedef std::vector<UINT> UINTVECTOR;
#ifdef _UNICODE
typedef std::wstring TSTRING;
#else
typedef std::string TSTRING;
#endif
// установите здесь ссылки на дополнительные заголовки, требующиеся для программы
