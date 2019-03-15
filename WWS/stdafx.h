// stdafx.h: включаемый файл для стандартных системных включаемых файлов
// или включаемых файлов для конкретного проекта, которые часто используются, но
// нечасто изменяются
//Этот файл относится к проекту WWS

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Исключите редко используемые компоненты из заголовков Windows
// Файлы заголовков Windows
#include <windows.h>
#include <vector>
#include <string>
typedef std::wstring WSTRING;
typedef std::vector<WSTRING> WSTRINGARRAY;
typedef std::string ASTRING;
typedef std::vector<ASTRING> ASTRINGARRAY;
// установите здесь ссылки на дополнительные заголовки, требующиеся для программы
