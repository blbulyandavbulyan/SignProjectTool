// BDL.cpp : Определяет экспортированные функции для приложения DLL.
//
/************************************Blbulyan Dialog Library***************************************************************
*данная бибилотека содержит основные диалоги и функции обработки их сообщений, используемые в программах Blbulyan Software*
**************************************************************************************************************************/
#include "stdafx.h"
#include "BDL.h"

BDLDLL_API INT_PTR CALLBACK BDL::EnterPasswordDlgProcA(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	HWND hEditPassword = GetDlgItem(hDlg, IDC_ENTERPASSWORDDLG_EDIT_PASSWORD), hShowPassword = GetDlgItem(hDlg, IDC_ENTERPASSWORDDLG_SHOW_PASSWORD), hClear = GetDlgItem(hDlg, IDC_DBL_ENTERPASSWORDCLEAR);
	static bool ShowPassword = true;
	static bool ClearButtonDisabled = true;
	static WCHAR PasswordChar = '\0';
	static HWND hToolTipForShowPassword = nullptr;
	static BDL::PEnterPasswordDlgInitParamA EPDI = nullptr;
	switch (message) {
		case WM_INITDIALOG:
		{
			EPDI = (PEnterPasswordDlgInitParamA)lParam;
			if (EPDI->hIconCaption != nullptr) {
				SendMessageA(hDlg, WM_SETICON, ICON_BIG, (LPARAM)EPDI->hIconCaption);
				SendMessageA(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)EPDI->hIconCaption);
			}
			if (EPDI->PasswordChar != '\0') {
				//данный участок кода поддерживает только Unicode версию символов, по этому ANSI символ конвертируется в Unicode
				STRING ForConvert; WSTRING Converted;
				ForConvert.push_back(EPDI->PasswordChar);
				Converted = BDL::strtowstr(ForConvert);
				if (Converted.size() > 0) {
					PasswordChar = Converted[0];
					SendMessageW(hEditPassword, EM_SETPASSWORDCHAR, (WPARAM)PasswordChar, NULL);
				}
			}
			else {
				PasswordChar = SendMessageW(hEditPassword, EM_GETPASSWORDCHAR, NULL, NULL);
			}
			if (EPDI->Caption != "") {
				DefDlgProcA(hDlg, WM_SETTEXT, NULL, (LPARAM)EPDI->Caption.c_str());
			}
			if (EPDI->EditPasswordCaption != "") {
				//данный участок кода не работает с ANSI строкамии по этому ANSI строка конвертируется в Unicode
				WSTRING EditPasswordCaption = BDL::strtowstr(EPDI->EditPasswordCaption);
				SendMessageW(hEditPassword, EM_SETCUEBANNER, TRUE, (LPARAM)EditPasswordCaption.c_str());
			}
			if (EPDI->OkButtonCaption != "") {
				HWND hOkButton = GetDlgItem(hDlg, IDOK);
				SetWindowTextA(hOkButton, EPDI->OkButtonCaption.c_str());
			}
			if (EPDI->CancelButtonCaption != "") {
				HWND hCancelButton = GetDlgItem(hDlg, IDCANCEL);
				SetWindowTextA(hCancelButton, EPDI->CancelButtonCaption.c_str());
			}
			HICON hShowPasswordIcon = LoadIconA(hThisDll, MAKEINTRESOURCEA(IDI_EYE_VISIBLE));
			SendMessageW(hShowPassword, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hShowPasswordIcon);
			if (EPDI->HasToolTip) {
				if (EPDI->ToolTipCaption[0] != "")hToolTipForShowPassword = BDL::CreateToolTipA(hShowPassword, hDlg, (PSTR)(EPDI->ToolTipCaption[0].c_str()));
				else {
					hToolTipForShowPassword = BDL::CreateToolTipA(hShowPassword, hDlg, (PSTR)"Показать пароль");
				}
			}
			return (INT_PTR)TRUE;
		}
		case WM_COMMAND:
			switch (HIWORD(wParam)) {
				case EN_CHANGE:
					switch (LOWORD(wParam)) {
						case IDC_ENTERPASSWORDDLG_EDIT_PASSWORD:{
							if (GetWindowTextLengthA((HWND)lParam) > 0){
								if (ClearButtonDisabled) {
									EnableWindow(hClear, TRUE);
									ClearButtonDisabled = false;
								}
							}
							else {
								ClearButtonDisabled = true;
								EnableWindow(hClear, FALSE);
							}
							break;
						}
						break;
					}
			}
			switch (LOWORD(wParam)) {
				case IDOK:{
					size_t textlength = GetWindowTextLengthA(hEditPassword) + 1;
					EPDI->Password->resize(textlength);
					GetWindowTextA(hEditPassword, EPDI->Password->data(), textlength);
				}
				case IDCANCEL:
					EPDI = nullptr; hToolTipForShowPassword = nullptr;
					ShowPassword = ClearButtonDisabled = true;
					EndDialog(hDlg, LOWORD(wParam));
					return (INT_PTR)TRUE;
				case IDC_ENTERPASSWORDDLG_SHOW_PASSWORD:{
					if (ShowPassword) {
						SendMessageW(hEditPassword, EM_SETPASSWORDCHAR, NULL, NULL);
						InvalidateRect(hEditPassword, NULL, TRUE);
						HICON hShowPasswordIcon = LoadIconA(hThisDll, MAKEINTRESOURCEA(IDI_EYE_NOT_VISIBLE));
						SendMessageA(hShowPassword, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hShowPasswordIcon);
						ShowPassword = false;
						if (hToolTipForShowPassword) {
							TOOLINFOA TI = { 0 };
							TI.hinst = hThisDll;
							TI.uId = (UINT_PTR)hShowPassword;
							TI.hwnd = hDlg;
							TI.cbSize = sizeof(TOOLINFOA);
							TI.lpszText = ((EPDI->ToolTipCaption[1] != "") ? (LPSTR)EPDI->ToolTipCaption[1].c_str() : (LPSTR)"Скрыть пароль");
							SendMessageA(hToolTipForShowPassword, TTM_UPDATETIPTEXTA, NULL, (LPARAM)&TI);
						}
					}
					else {
						SendMessageW(hEditPassword, EM_SETPASSWORDCHAR, (WPARAM)PasswordChar, NULL);
						InvalidateRect(hEditPassword, NULL, TRUE);
						HICON hShowPasswordIcon = LoadIconA(hThisDll, MAKEINTRESOURCEA(IDI_EYE_VISIBLE));
						SendMessageA(hShowPassword, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hShowPasswordIcon);
						ShowPassword = true;
						if (hToolTipForShowPassword != nullptr) {
							TOOLINFOA TI = { 0 };
							TI.hinst = hThisDll;
							TI.uId = (UINT_PTR)hShowPassword;
							TI.hwnd = hDlg;
							TI.cbSize = sizeof(TOOLINFOA);
							TI.lpszText = ((EPDI->ToolTipCaption[0] != "") ? (LPSTR)EPDI->ToolTipCaption[0].c_str() : (LPSTR)"Показать пароль");
							SendMessageA(hToolTipForShowPassword, TTM_UPDATETIPTEXTA, NULL, (LPARAM)&TI);
						}
					}
					break;
				}
				case IDC_DBL_ENTERPASSWORDCLEAR:
					SetDlgItemTextA(hDlg, IDC_ENTERPASSWORDDLG_EDIT_PASSWORD, "");
					break;
			}
	}
	return (INT_PTR)FALSE;
}
BDLDLL_API INT_PTR CALLBACK BDL::EnterPasswordDlgProcW(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	HWND hEditPassword = GetDlgItem(hDlg, IDC_ENTERPASSWORDDLG_EDIT_PASSWORD), hShowPassword = GetDlgItem(hDlg, IDC_ENTERPASSWORDDLG_SHOW_PASSWORD), hClear = GetDlgItem(hDlg, IDC_DBL_ENTERPASSWORDCLEAR);
	static bool ShowPassword = true;
	static bool ClearButtonDisabled = true;
	static WCHAR PasswordChar = '\0';
	static HWND hToolTipForShowPassword = nullptr;
	static BDL::PEnterPasswordDlgInitParamW EPDI = nullptr;
	switch (message) {
		case WM_INITDIALOG:{
			EPDI = (PEnterPasswordDlgInitParamW)lParam;
			if (EPDI->hIconCaption != nullptr) {
				SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)EPDI->hIconCaption);
				SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)EPDI->hIconCaption);
			}
			if (EPDI->PasswordChar != L'\0') {
				PasswordChar = EPDI->PasswordChar;
				SendMessageW(hEditPassword, EM_SETPASSWORDCHAR, (WPARAM)PasswordChar, NULL);
			}
			else {
				PasswordChar = (WCHAR)SendMessage(hEditPassword, EM_GETPASSWORDCHAR, NULL, NULL);
			}
			if (EPDI->Caption != L"") {
				DefDlgProcW(hDlg, WM_SETTEXT, NULL, (LPARAM)EPDI->Caption.c_str());
			}
			if (EPDI->EditPasswordCaption != L"") {
				SendMessageW(hEditPassword, EM_SETCUEBANNER, TRUE, (LPARAM)EPDI->EditPasswordCaption.c_str());
			}
			if (EPDI->OkButtonCaption != L"") {
				HWND hOkButton = GetDlgItem(hDlg, IDOK);
				SetWindowTextW(hOkButton, EPDI->OkButtonCaption.c_str());
			}
			if (EPDI->CancelButtonCaption != L"") {
				HWND hCancelButton = GetDlgItem(hDlg, IDCANCEL);
				SetWindowTextW(hCancelButton, EPDI->CancelButtonCaption.c_str());
			}
			HICON hShowPasswordIcon = LoadIconW(hThisDll, MAKEINTRESOURCEW(IDI_EYE_VISIBLE));
			SendMessageW(hShowPassword, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hShowPasswordIcon);
			if (EPDI->HasToolTip) {
				if (EPDI->ToolTipCaption[0] != L"")hToolTipForShowPassword = BDL::CreateToolTipW(hShowPassword, hDlg, (PWSTR)(EPDI->ToolTipCaption[0].c_str()));
				else {
					hToolTipForShowPassword = BDL::CreateToolTipW(hShowPassword, hDlg, (PWSTR)L"Показать пароль");
				}
			}
			return (INT_PTR)TRUE;
		}
		case WM_COMMAND:
			switch (HIWORD(wParam)) {
				case EN_CHANGE:
					switch (LOWORD(wParam)) {
						case IDC_ENTERPASSWORDDLG_EDIT_PASSWORD:
						{
							if (GetWindowTextLengthW((HWND)lParam)/sizeof(WCHAR) > 0) {
								if (ClearButtonDisabled) {
									EnableWindow(hClear, TRUE);
									ClearButtonDisabled = false;
								}
							}
							else {
								ClearButtonDisabled = true;
								EnableWindow(hClear, FALSE);
							}
							break;
						}
						break;
					}
			}
			switch (LOWORD(wParam)) {
				case IDOK:{
					size_t textlength = GetWindowTextLengthW(hEditPassword) + 1;
					EPDI->Password->resize(textlength);
					GetWindowTextW(hEditPassword, EPDI->Password->data(), textlength);
				}
				case IDCANCEL:
					EPDI = nullptr; hToolTipForShowPassword = nullptr;
					ShowPassword = true;
					EndDialog(hDlg, LOWORD(wParam));
					return (INT_PTR)TRUE;
				case IDC_ENTERPASSWORDDLG_SHOW_PASSWORD:{
					if (ShowPassword) {
						SendMessageW(hEditPassword, EM_SETPASSWORDCHAR, NULL, NULL);
						InvalidateRect(hEditPassword, NULL, TRUE);
						HICON hShowPasswordIcon = LoadIconW(hThisDll, MAKEINTRESOURCEW(IDI_EYE_NOT_VISIBLE));
						SendMessageW(hShowPassword, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hShowPasswordIcon);
						ShowPassword = false;
						if (hToolTipForShowPassword) {
							TOOLINFOW TI = { 0 };
							TI.hinst = hThisDll;
							TI.uId = (UINT_PTR)hShowPassword;
							TI.hwnd = hDlg;
							TI.cbSize = sizeof(TOOLINFOW);
							TI.lpszText = ((EPDI->ToolTipCaption[1] != L"") ? (LPWSTR)EPDI->ToolTipCaption[1].c_str() : (LPWSTR)L"Скрыть пароль");
							SendMessageW(hToolTipForShowPassword, TTM_UPDATETIPTEXTW, NULL, (LPARAM)&TI);
						}
					}
					else {
						SendMessageW(hEditPassword, EM_SETPASSWORDCHAR, (WPARAM)PasswordChar, NULL);
						InvalidateRect(hEditPassword, NULL, TRUE);
						HICON hShowPasswordIcon = LoadIconW(hThisDll, MAKEINTRESOURCEW(IDI_EYE_VISIBLE));
						SendMessageW(hShowPassword, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hShowPasswordIcon);
						ShowPassword = true;
						if (hToolTipForShowPassword != nullptr) {
							TOOLINFOW TI = { 0 };
							TI.hinst = hThisDll;
							TI.uId = (UINT_PTR)hShowPassword;
							TI.hwnd = hDlg;
							TI.cbSize = sizeof(TOOLINFOW);
							TI.lpszText = ((EPDI->ToolTipCaption[0] != L"") ? (LPWSTR)EPDI->ToolTipCaption[0].c_str() : (LPWSTR)L"Показать пароль");
							SendMessageW(hToolTipForShowPassword, TTM_UPDATETIPTEXTW, NULL, (LPARAM)&TI);
						}
					}
					break;
				}
				case IDC_DBL_ENTERPASSWORDCLEAR:
					SetDlgItemTextA(hDlg, IDC_ENTERPASSWORDDLG_EDIT_PASSWORD, "");
					break;
			}
	}
	return (INT_PTR)FALSE;
}
WSTRING BDL::strtowstr(const STRING &str) {
	// Convert an ASCII string to a Unicode String 
	WSTRING wstrTo;
	WCHAR *wszTo = new WCHAR[str.length() + 1];
	wszTo[str.size()] = L'\0';
	MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wszTo, (int)str.length());
	wstrTo = wszTo;
	delete[] wszTo;
	return wstrTo;
}
HWND BDL::CreateToolTipA(HWND hDlgItem, HWND hDlg, LPSTR pszText) {
	if (!hDlgItem || !hDlg || !pszText) {
		return FALSE;
	}
	// Get the window of the tool.
	// Create the tooltip. g_hInst is the global instance handle.
	HWND hwndTip = CreateWindowExA(NULL, TOOLTIPS_CLASSA, NULL, WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hDlg, NULL, hThisDll, NULL);
	if (!hDlgItem || !hwndTip)return (HWND)NULL;
	// Associate the tooltip with the tool.
	TOOLINFOA toolInfo = { 0 };
	toolInfo.cbSize = sizeof(toolInfo);
	toolInfo.hwnd = hDlg;
	toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	toolInfo.uId = (UINT_PTR)hDlgItem;
	toolInfo.lpszText = pszText;
	SendMessageA(hwndTip, TTM_ADDTOOLA, 0, (LPARAM)&toolInfo);
	return hwndTip;
}

HWND BDL::CreateToolTipW(HWND hDlgItem, HWND hDlg, PWSTR pszText) {
	if (!hDlgItem || !hDlg || !pszText) {
		return FALSE;
	}
	// Get the window of the tool.
	// Create the tooltip. g_hInst is the global instance handle.
	HWND hwndTip = CreateWindowExW(NULL, TOOLTIPS_CLASSW, NULL, WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hDlg, NULL, hThisDll, NULL);
	if (!hDlgItem || !hwndTip)return (HWND)NULL;
	// Associate the tooltip with the tool.
	TOOLINFOW toolInfo = { 0 };
	toolInfo.cbSize = sizeof(toolInfo);
	toolInfo.hwnd = hDlg;
	toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	toolInfo.uId = (UINT_PTR)hDlgItem;
	toolInfo.lpszText = pszText;
	SendMessageW(hwndTip, TTM_ADDTOOLW, 0, (LPARAM)&toolInfo);
	return hwndTip;
}
