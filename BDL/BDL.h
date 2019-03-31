#pragma once
/*******************************************Blbulyan Dialog Library********************************************************
*������ ���������� �������� �������� ������� � ������� ��������� �� ���������, ������������ � ���������� Blbulyan Software*
**************************************************************************************************************************/
#include "stdafx.h"
#include "resource.h"
#ifdef BDL_EXPORTS
#define BDLDLL_API __declspec(dllexport)
#else
#define BDLDLL_API __declspec(dllimport)
#endif
#ifdef BDL_ENABLE_PROTOTYPES
namespace BDL {
	/*Unicode DlgProc`s versions*/
	BDLDLL_API INT_PTR CALLBACK EnterPasswordDlgProcW(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);//������� ��������� ��������� ��� ������� IDDBDL_ENTERPASSWORD
	#ifdef BDL_EXPORTS
	/*Unicode ������ ��������������� �������*/
	HWND CreateToolTipW(HWND hDlgItem, HWND hDlg, LPCWSTR pszText, HINSTANCE hInst);//������� ������ ����������� ����������
	bool CompareTwoLines(LPCWSTR str1, LPCWSTR str2);
	#endif
}
#endif
namespace BDL {
	typedef struct EnterPasswordDlgInitParamW {//��������� ����������������� ���������� ��� ������� EnterPasswordDlgProcW
		LPWSTR *Password = nullptr;//��������� �� ������, � ������� ���������� �������� ������
		LPCWSTR Caption = nullptr;//��������� ����������� ���� ����� ������ 
		LPCWSTR EditPasswordCaption = nullptr;//���������, ������������ � ���� ����� ������
		LPCWSTR OkButtonCaption = nullptr;//�����, ������� ������������ � ������ ID�� ���������� �������
		LPCWSTR CancelButtonCaption = nullptr;//�����, ������� ������������ � ������ IDCANCEL ���������� ������
		LPCWSTR ClearButtonCaption = nullptr;//����� ������������ � ������ IDC_DBL_ENTERPASSWORDCLEAR
		LPCWSTR ToolTipCaption[2] = {
			nullptr,
			nullptr
		};//������ �� ��������, ������� ����� �������������� � �����������, �������� �� ��������� ����� �������������� � ��� ������, ���� �������������� ������ - ������
		//�������� ������� �������:
		// ToolTipCaption[0] - ����� ����������, ������� ������������ ��� ������� IDC_ENTERPASSWORDDLG_SHOW_PASSWORD, ����� � �������� "�������� ������", �� ��������� �������� ���� ���������� "�������� ������"
		//ToolTipCaption[1] - ����� ����������, ������� ������������ ��� ������� IDC_ENTERPASSWORDDLG_SHOW_PASSWORD, ����� � �������� "������ ������", �� ��������� �������� ���� ���������� "������ ������"
		WCHAR PasswordChar = '\0';//������, ������������ ��� ����� ������, ���� ������ ����� ������������� ������ �� ��������� ����������� �������� '\0'
		HICON hIconCaption = nullptr;// ���������� ������ ������������ � ��������� ����������� ����
		BOOL HasToolTip = FALSE;//������ ���������� ���������� ����� �� ������ ����� ����������� ����������
	} *PEnterPasswordDlgInitParamW;
}