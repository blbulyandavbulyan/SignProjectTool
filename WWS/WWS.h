#pragma once
#include "stdafx.h"
#ifdef WWS_EXPORTS
#define MYDLLLIBRARY_API __declspec(dllexport)
#else
#define MYDLLLIBRARY_API __declspec(dllimport)
#endif
//Unicode Version
MYDLLLIBRARY_API WSTRINGARRAY BreakAStringIntoAnArrayOfStringsByCharacter(WSTRING StringToBreak, WCHAR CharacterToBreak);// ������� ��������� ������ �� ������ ����� �� ������� CharacterToBreak
MYDLLLIBRARY_API WSTRING DeleteTwoCharactersInARow(WSTRING StringForDeleteTwoCharactersInARow, WCHAR SymbolForDelete);//������� ������� ������������� ������ (��������� ������ ���� ��� �����)
MYDLLLIBRARY_API void MessageError(WSTRING ErrorText, WSTRING ErrorCaption, HWND hWnd);// ������� ������� ���������� ���� � ������� (������� �������� ��� ������ � ������� GetLastError)
MYDLLLIBRARY_API void MessageError(WSTRING ErrorText, WSTRING ErrorCaption, HWND hWnd, HRESULT hErrorCode);//������� ������� ���������� ���� � ������� �� ��������� hErrorCode
// ASCII version
MYDLLLIBRARY_API ASTRINGARRAY BreakAStringIntoAnArrayOfStringsByCharacter(ASTRING StringToBreak, CHAR CharacterToBreak);// ������� ��������� ������ �� ������ ����� �� ������� CharacterToBreak
MYDLLLIBRARY_API ASTRING DeleteTwoCharactersInARow(ASTRING StringForDeleteTwoCharactersInARow, CHAR SymbolForDelete);//������� ������� ������������� ������ (��������� ������ ���� ��� �����)
MYDLLLIBRARY_API void MessageError(ASTRING ErrorText, ASTRING ErrorCaption, HWND hWnd);// ������� ������� ���������� ���� � ������� (������� �������� ��� ������ � ������� GetLastError)
MYDLLLIBRARY_API void MessageError(ASTRING ErrorText, ASTRING ErrorCaption, HWND hWnd, HRESULT hErrorCode);//������� ������� ���������� ���� � ������� �� ��������� hErrorCode