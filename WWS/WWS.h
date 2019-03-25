#pragma once
#include "stdafx.h"
#ifdef WWS_EXPORTS
#define WWSDLL_API __declspec(dllexport)
#else
#define WWSDLL_API __declspec(dllimport)
#endif
/*****************************************************
**��������� ����������� ��� ������ ������� � ��� DLL**
*****************************************************/
//UNICODE Structures Version
//������ ��������� ����� ��� ������ Unicode ������ ������� ErrorString
struct WWS_ERROR_REPORT_STRUCTW {
	WSTRING ErrorString;//������ � �������
	WSTRING ErrorCaption;//��������� ������
	DWORD ErrorCode;// ��� ������
};
//ANSI Structures Version
//������ ��������� ����� ��� ������ ANSI ������ ������� ErrorString
struct WWS_ERROR_REPORT_STRUCTA {
	ASTRING ErrorString;//������ � �������
	ASTRING ErrorCaption;//��������� ������
	DWORD ErrorCode;// ��� ������
};
//Unicode Version
WWSDLL_API WSTRINGARRAY BreakAStringIntoAnArrayOfStringsByCharacter(WSTRING StringToBreak, WCHAR CharacterToBreak);// ������� ��������� ������ �� ������ ����� �� ������� CharacterToBreak
WWSDLL_API WSTRING DeleteTwoCharactersInARow(WSTRING StringForDeleteTwoCharactersInARow, WCHAR SymbolForDelete);//������� ������� ������������� ������ (��������� ������ ���� ��� �����)
WWSDLL_API int MessageError(LPCWSTR ErrorText, LPCWSTR ErrorCaption, HWND hWnd);// ������� ������� ���������� ���� � ������� (������� �������� ��� ������ � ������� GetLastError)
WWSDLL_API int MessageError(LPCWSTR ErrorText, LPCWSTR ErrorCaption, HWND hWnd, HRESULT hErrorCode);//������� ������� ���������� ���� � ������� �� ��������� hErrorCode
WWSDLL_API void MessageDebug(LPCWSTR ErrorText, LPCWSTR ErrorCaption);//������� ������� ��������� �� ������ � ������� ��������� (���� �������� ��� ������ � ������� GetLastError)
WWSDLL_API void MessageDebug(LPCWSTR ErrorText, LPCWSTR ErrorCaption, HRESULT hErrorCode);//������� ������� ��������� �� ������ � ������� ��������� (������� ��� ������ HRESULT)
WWSDLL_API WSTRING ErrorString(LPCWSTR ErrorText, HRESULT hErrorCode);//������� ��������� ��������� �� ������ �� ��������� HRESULT � �������� ��� � ��������� WWS_ERROR_REPORT_STRUCTW 
WWSDLL_API WWS_ERROR_REPORT_STRUCTW ErrorString(LPCWSTR ErrorText);//������� ��������� ��������� �� ������ (���� �������� ��� ������) � �������� ��� � ��������� WWS_ERROR_REPORT_STRUCTW 
// ANSI version
WWSDLL_API ASTRINGARRAY BreakAStringIntoAnArrayOfStringsByCharacter(ASTRING StringToBreak, CHAR CharacterToBreak);// ������� ��������� ������ �� ������ ����� �� ������� CharacterToBreak
WWSDLL_API ASTRING DeleteTwoCharactersInARow(ASTRING StringForDeleteTwoCharactersInARow, CHAR SymbolForDelete);//������� ������� ������������� ������ (��������� ������ ���� ��� �����)
WWSDLL_API int MessageError(LPCSTR ErrorText, LPCSTR ErrorCaption, HWND hWnd);// ������� ������� ���������� ���� � ������� (������� �������� ��� ������ � ������� GetLastError)
WWSDLL_API int MessageError(LPCSTR ErrorText, LPCSTR ErrorCaption, HWND hWnd, HRESULT hErrorCode);//������� ������� ���������� ���� � ������� �� ��������� hErrorCode
WWSDLL_API void MessageDebug(LPCSTR ErrorText, LPCSTR ErrorCaption);//������� ������� ��������� �� ������ � ������� ��������� (���� �������� ��� ������ � ������� GetLastError)
WWSDLL_API void MessageDebug(LPCSTR ErrorText, LPCSTR ErrorCaption, HRESULT hErrorCode);//������� ������� ��������� �� ������ � ������� ��������� (������� ��� ������ HRESULT)
WWSDLL_API ASTRING ErrorString(LPCSTR ErrorText, HRESULT hErrorCode);//������� ��������� ��������� �� ������ �� ��������� HRESULT � �������� ��� � ��������� WWS_ERROR_REPORT_STRUCTA 
WWSDLL_API WWS_ERROR_REPORT_STRUCTA ErrorString(LPCSTR ErrorText);//������� ��������� ��������� �� ������ (���� �������� ��� ������) � �������� ��� � ��������� WWS_ERROR_REPORT_STRUCTA