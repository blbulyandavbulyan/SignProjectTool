// dllmain.cpp : Определяет точку входа для приложения DLL.
/**********************************************Blbulyan Dialog Library*****************************************************
*данная бибилотека содержит основные диалоги и функции обработки их сообщений, используемые в программах Blbulyan Software*
**************************************************************************************************************************/
#include "stdafx.h"
HMODULE hThisDll = nullptr;
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
		hThisDll = hModule;
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

