#pragma once
#include "stdafx.h"
#ifdef WWS_EXPORTS
#define WWSDLL_API __declspec(dllexport)
#else
#define WWSDLL_API __declspec(dllimport)
#endif

//UNICODE Structures Version
//данная структура нужна для работы Unicode версии функции ErrorString
namespace WWS {
	/*****************************************************
	**Структуры необходимые для работы функций в это DLL**
	*****************************************************/
	struct WWS_ERROR_REPORT_STRUCTW {
		WSTRING ErrorString;//строка с ошибкой
		WSTRING ErrorCaption;//заголовок ошибки
		DWORD ErrorCode;// код ошибки
	};
	//Unicode Version
	WWSDLL_API WSTRINGARRAY BreakAStringIntoAnArrayOfStringsByCharacter(WSTRING StringToBreak, WCHAR CharacterToBreak);// функция разбивает строку на массив строк по символу CharacterToBreak
	WWSDLL_API WSTRING DeleteTwoCharactersInARow(WSTRING StringForDeleteTwoCharactersInARow, WCHAR SymbolForDelete);//функция удаляет повторяющийся символ (оставляет только одну его копию)
	WWSDLL_API int MessageError(LPCWSTR ErrorText, LPCWSTR ErrorCaption, HWND hWnd);// функция выводит диалоговое окно с ошибкой (функция получает код ошибки с помощью GetLastError)
	WWSDLL_API int MessageError(LPCWSTR ErrorText, LPCWSTR ErrorCaption, HWND hWnd, HRESULT hErrorCode);//функция выводит диалоговое окно с ошибкой по заданному hErrorCode
	WWSDLL_API void MessageDebug(LPCWSTR ErrorText, LPCWSTR ErrorCaption);//функция выводит сообщение об ошибке в консоль отладчика (сама получает код ошибки с помощью GetLastError)
	WWSDLL_API void MessageDebug(LPCWSTR ErrorText, LPCWSTR ErrorCaption, HRESULT hErrorCode);//функция выводит сообщение об ошибке в консоль отладчика (требует код ошибки HRESULT)
	WWSDLL_API WSTRING ErrorString(LPCWSTR ErrorText, HRESULT hErrorCode);//функция формирует сообщение об ошибке по заданному HRESULT и помещает его в структуру WWS_ERROR_REPORT_STRUCTW 
	WWSDLL_API WWS_ERROR_REPORT_STRUCTW ErrorString(LPCWSTR ErrorText);//функция формирует сообщение об ошибке (сама получает код ошибки) и помещает его в структуру WWS_ERROR_REPORT_STRUCTW 
}