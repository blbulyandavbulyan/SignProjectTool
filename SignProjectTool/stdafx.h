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
#include <wincrypt.h>
#include <cryptuiapi.h>
#include <shellapi.h>
//#include <Uxtheme.h>
#include <ktmw32.h>
// Файлы заголовков среды выполнения C
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <bitset>
//#include <iostream>
//Файлы заголовков проектных DLL
#include "../WWS/WWS.h"
#define BDL_ENABLE_PROTOTYPES
#include "../BDL/BDL.h"
//файлы заголовков этого проекта
/*ВНИМАНИЕ! СОБЛЮДАТЬ ЗАДАННЫЙ ПОРЯДОК ВКЛЮЧЕНИЯ ЗАГОЛОВОЧНЫХ ФАЙЛОВ*/
#include "SignProjectTool.h"
#include "sign.h"
#include "files.h"
#include "dlgprocs.h"
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
