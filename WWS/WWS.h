#pragma once
#include "stdafx.h"
#ifdef WWS_EXPORTS
#define WWSDLL_API __declspec(dllexport)
#else
#define WWSDLL_API __declspec(dllimport)
#endif

//UNICODE Structures Version
//������ ��������� ����� ��� ������ Unicode ������ ������� ErrorString
namespace WWS {
	/*****************************************************
	**��������� ����������� ��� ������ ������� � ��� DLL**
	*****************************************************/
	struct WWS_ERROR_REPORT_STRUCTW {
		WSTRING ErrorString;//������ � �������
		WSTRING ErrorCaption;//��������� ������
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
}