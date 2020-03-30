#pragma once
/**************************************************************
***������ ��������� �������� ����������� ���������� ��������***
**************************************************************/
INT_PTR CALLBACK SettingsTimeStampsServersDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);//��������� ��������� ��������� ������� IDD_SETTINGS_TIMESTAMP_SERVERS
INT_PTR CALLBACK SettingsForActionsPerformedAfterSigningDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);//��������� ��������� ��������� ������� IDD_SETTINGS_FOR_ACTIONS_PERFORMED_AFTER_SIGNING
UINT CalcBItemWidth(HWND hWnd, LPCTSTR Text);//������� ������� ������ ������ � ��������, ���������� ����� ����������� ������ � ListBox, ����� ������ ������ ���������� � �������� �������� �������������� ���������
INT StringExistInListBox(HWND hListBox, WSTRING str);//���������� �� ������ � ListBox
INT_PTR CALLBACK StartConfigurationDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);// ��������� ��������� ��������� ������� IDD_START_CONFIGURATION
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);// ���������� ��������� ��� ���� "� ���������".
UINT CalcBItemWidth(HWND hWnd, LPCTSTR Text);//������ ������� ������������ ������ ������ ��������
INT StringExistInListBox(HWND hListBox, WSTRING str);//������ ������� ��������� ���������� �� ������ � ListBox
//������ ��������� "����������" ������ ��������� ������������
struct START_CONFIGURATION_RESULT {
	WSTRING CertificateFileName;
	ALG_ID HashAlgoritmId = NULL;
	WSTRINGARRAY TimeStampServers;
	typedef START_CONFIGURATION_RESULT* PSTART_CONFIRURATION_RESULT;
};