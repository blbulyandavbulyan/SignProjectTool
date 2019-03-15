#pragma once
#include "stdafx.h"
#ifdef WWS_EXPORTS
#define MYDLLLIBRARY_API __declspec(dllexport)
#else
#define MYDLLLIBRARY_API __declspec(dllimport)
#endif
//Unicode Version
MYDLLLIBRARY_API WSTRINGARRAY BreakAStringIntoAnArrayOfStringsByCharacter(WSTRING StringToBreak, WCHAR CharacterToBreak);// функция разбивает строку на массив строк по символу CharacterToBreak
MYDLLLIBRARY_API WSTRING DeleteTwoCharactersInARow(WSTRING StringForDeleteTwoCharactersInARow, WCHAR SymbolForDelete);//функция удаляет повторяющийся символ (оставляет только одну его копию)
MYDLLLIBRARY_API void MessageError(WSTRING ErrorText, WSTRING ErrorCaption, HWND hWnd);// функция выводит диалоговое окно с ошибкой (функция получает код ошибки с помощью GetLastError)
MYDLLLIBRARY_API void MessageError(WSTRING ErrorText, WSTRING ErrorCaption, HWND hWnd, HRESULT hErrorCode);//функция выводит диалоговое окно с ошибкой по заданному hErrorCode
// ASCII version
MYDLLLIBRARY_API ASTRINGARRAY BreakAStringIntoAnArrayOfStringsByCharacter(ASTRING StringToBreak, CHAR CharacterToBreak);// функция разбивает строку на массив строк по символу CharacterToBreak
MYDLLLIBRARY_API ASTRING DeleteTwoCharactersInARow(ASTRING StringForDeleteTwoCharactersInARow, CHAR SymbolForDelete);//функция удаляет повторяющийся символ (оставляет только одну его копию)
MYDLLLIBRARY_API void MessageError(ASTRING ErrorText, ASTRING ErrorCaption, HWND hWnd);// функция выводит диалоговое окно с ошибкой (функция получает код ошибки с помощью GetLastError)
MYDLLLIBRARY_API void MessageError(ASTRING ErrorText, ASTRING ErrorCaption, HWND hWnd, HRESULT hErrorCode);//функция выводит диалоговое окно с ошибкой по заданному hErrorCode