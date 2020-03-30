#pragma once
/**************************************************************
***Данный заголовок содержит определение диалоговых процедур***
**************************************************************/
INT_PTR CALLBACK SettingsTimeStampsServersDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);//процедура обработки сообщений диалога IDD_SETTINGS_TIMESTAMP_SERVERS
INT_PTR CALLBACK SettingsForActionsPerformedAfterSigningDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);//процедура обработки сообщений диалога IDD_SETTINGS_FOR_ACTIONS_PERFORMED_AFTER_SIGNING
UINT CalcBItemWidth(HWND hWnd, LPCTSTR Text);//функция считает размер строки в пикселях, вызывается перед добавлением строки в ListBox, затем данный размер использует в качестве значения горизонтальной прокрутки
INT StringExistInListBox(HWND hListBox, WSTRING str);//существует ли строка в ListBox
INT_PTR CALLBACK StartConfigurationDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);// процедура обработки сообщений диалога IDD_START_CONFIGURATION
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);// Обработчик сообщений для окна "О программе".
UINT CalcBItemWidth(HWND hWnd, LPCTSTR Text);//данная функция подсчитывает длинну полосы прокрутк
INT StringExistInListBox(HWND hListBox, WSTRING str);//данная функция проверяет существует ли строка в ListBox
//данную структуру "возвращает" диалог стартовой конфигурации
struct START_CONFIGURATION_RESULT {
	WSTRING CertificateFileName;
	ALG_ID HashAlgoritmId = NULL;
	WSTRINGARRAY TimeStampServers;
	typedef START_CONFIGURATION_RESULT* PSTART_CONFIRURATION_RESULT;
};