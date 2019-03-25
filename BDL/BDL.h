#pragma once
/*******************************************Blbulyan Dialog Library********************************************************
*������ ���������� �������� �������� ������� � ������� ��������� �� ���������, ������������ � ���������� Blbulyan Software*
**************************************************************************************************************************/
#include "stdafx.h"
#include "resource.h"
#ifdef BDL_EXPORTS
#define BDLDLL_API __declspec(dllexport)
//#pragma comment(linker, "/EXPORT:EnterPasswordDlgProcA@1=EnterPasswordDlgProcA")
//#pragma comment(linker, "/EXPORT:EnterPasswordDlgProcW@2=EnterPasswordDlgProcW")
#else
#define BDLDLL_API __declspec(dllimport)
#endif
#ifdef BDL_ENABLE_PROTOTYPES
namespace BDL {
	BDLDLL_API INT_PTR CALLBACK EnterPasswordDlgProcA(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	BDLDLL_API INT_PTR CALLBACK EnterPasswordDlgProcW(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	#ifdef BDL_EXPORTS
	HWND CreateToolTipA(HWND hDlgItem, HWND hDlg, LPSTR pszText);
	HWND CreateToolTipW(HWND hDlgItem, HWND hDlg, PWSTR pszText);
	WSTRING strtowstr(const STRING &str);
	#endif
}
#endif
namespace BDL {
	typedef struct EnterPasswordDlgInitParamA {//��������� ����������������� ���������� ��� ������� EnterPasswordDlgProcA
		CHARVECTOR *Password;//��������� �� ������, � ������� ���������� �������� ������
		STRING Caption;//��������� ����������� ���� ����� ������ 
		STRING EditPasswordCaption;//���������, ������������ � ���� ����� ������
		STRING OkButtonCaption;//�����, ������� ������������ � ������ ID�� ���������� �������
		STRING CancelButtonCaption;//�����, ������� ������������ � ������ IDCANCEL ���������� ������
		STRING ToolTipCaption[2];//������ �� ��������, ������� ����� �������������� � �����������, �������� �� ��������� ����� �������������� � ��� ������, ���� �������������� ������ - ������
		//�������� ������� �������:
		// ToolTipCaption[0] - ����� ����������, ������� ������������ ��� ������� "��������/������ ������", ����� � �������� "�������� ������", �� ��������� �������� ���� ���������� "�������� ������"
		//ToolTipCaption[1] - ����� ����������, ������� ������������ ��� ������� "��������/������ ������", ����� � �������� "������ ������", �� ��������� �������� ���� ���������� "������ ������"
		CHAR PasswordChar = '\0';//������, ������������ ��� ����� ������, ���� ������ ����� ������������� ������ �� ��������� ����������� �������� '\0'
		HICON hIconCaption = nullptr;// ���������� ������ ������������ � ��������� ����������� ����
		bool HasToolTip = false;//������ ���������� ���������� ����� �� ������ ����� ����������� ����������
	} *PEnterPasswordDlgInitParamA;
	typedef struct EnterPasswordDlgInitParamW {//��������� ����������������� ���������� ��� ������� EnterPasswordDlgProcW
		WCHARVECTOR *Password;//��������� �� ������, � ������� ���������� �������� ������
		WSTRING Caption;//��������� ����������� ���� ����� ������ 
		WSTRING EditPasswordCaption;//���������, ������������ � ���� ����� ������
		WSTRING OkButtonCaption;//�����, ������� ������������ � ������ ID�� ���������� �������
		WSTRING CancelButtonCaption;//�����, ������� ������������ � ������ IDCANCEL ���������� ������
		WSTRING ToolTipCaption[2];//������ �� ��������, ������� ����� �������������� � �����������, �������� �� ��������� ����� �������������� � ��� ������, ���� �������������� ������ - ������
		//�������� ������� �������:
		// ToolTipCaption[0] - ����� ����������, ������� ������������ ��� ������� IDC_ENTERPASSWORDDLG_SHOW_PASSWORD, ����� � �������� "�������� ������", �� ��������� �������� ���� ���������� "�������� ������"
		//ToolTipCaption[1] - ����� ����������, ������� ������������ ��� ������� IDC_ENTERPASSWORDDLG_SHOW_PASSWORD, ����� � �������� "������ ������", �� ��������� �������� ���� ���������� "������ ������"
		WCHAR PasswordChar = '\0';//������, ������������ ��� ����� ������, ���� ������ ����� ������������� ������ �� ��������� ����������� �������� '\0'
		HICON hIconCaption = nullptr;// ���������� ������ ������������ � ��������� ����������� ����
		bool HasToolTip = false;//������ ���������� ���������� ����� �� ������ ����� ����������� ����������
	} *PEnterPasswordDlgInitParamW;
}