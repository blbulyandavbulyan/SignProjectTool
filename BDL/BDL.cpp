// BDL.cpp : Определяет экспортированные функции для приложения DLL.
//
/************************************Blbulyan Dialog Library***************************************************************
*данная бибилотека содержит основные диалоги и функции обработки их сообщений, используемые в программах Blbulyan Software*
**************************************************************************************************************************/
#include "stdafx.h"
#include "BDL.h"
/*
Маленькое пояснение, в дальнейшем в тексте может быть использована фраза: "значение кнопки %ИД_КНОПКИ% %ЗНАЧЕНИЕ%" это фраза означает что кнопка с %ИД_КНОПКИ% выполняет действие %ЗНАЧЕНИЕ%
Логика работы функций обработки сообщений от диалога IDDBDL_ENTERPASSWORD следующая:
Когда приходит сообщение WM_INITDIALOG функция преобразует lParam к указателю на свою инициализационную структуру
!ВАЖНО ЧТОБЫ ОН БЫЛ ПЕРЕДАН!
ДИАЛОГ ДОЛЖЕН СОЗДАВАТЬСЯ С ПОМОЩЬЮ ФУНКЦИИ DialogBoxParamA для функции обработки сообщений BDL::EnterPasswordDlgProcA или DialogBoxParamW для функции BDL::EnterPasswordDlgProcW!
ВАЖНО ТАК ЖЕ ЧТОБЫ в lParam действительно содержался указатель на требуемую структуру, ответсвенность за вызов функций и за передачу в них требуемых параметров берёт на себя 
вызовщик, передача lParam, содержащего недействительный указатель на инициализационную структуру, может привести к краху вашего приложения
После того как lParam был преобразован в указатель на инициализационную структуру, начинается процесс инициализации, включающий следующие пункты:
	НАЧАЛО ИНИЦИАЛИЗАЦИИ:
	1)Установка иконки диалогового окна, если её указатель не равен nullptr
	2)Установка символа, отображаемого вместо пароля, если он установлен в значение '\0' то используется символ по умолчанию
	3)Установка заголовка диалогового окна, если в инициализационной структуре он не пустой, то используем его, в противном случае, заголовок по умолчанию
	4)Установка текста для кнопки IDOK, если соотвествующий член в инициализационной структуре не пустой, то используем его, в противном случае заголовок по умолчанию
	5)Установка текста для кнопки IDCANCEL, если соотвествующий член в инициализационной структуре не пустой, то используем его, в противном случае заголовок по умолчанию
	6)Установка текста для кнопки IDC_DBL_ENTERPASSWORDCLEAR, если соотвествующий член в инициализационной структуре не пустой, то используем его, в противном случае заголовок по умолчанию
	7)Загрузка и установка иконки для кнопки IDC_ENTERPASSWORDDLG_SHOW_PASSWORD, по умолчанию, кнопка в начальный момент инициализации имеет значение показать пароль, по этому загружается соотвествующая иконка
	8)Если член структуры, отвечающий за то, показывать ли всплывающие подсказски установлен в true, тогда происходит следующее:
		Описание инициализации подсказок для кнопки IDC_ENTERPASSWORDDLG_SHOW_PASSWORD
		Так как в данном диалоговом окне единственная подсказска, которая показывается над кнопкой IDC_ENTERPASSWORDDLG_SHOW_PASSWORD,
		то в момент инициализации проверяется что соотвествующий член массива с текстом подсказок не пустой(предназначенный для кнопки IDC_ENTERPASSWORDDLG_SHOW_PASSWORD, имеющей значение показать пароль),
		если это так, то в качестве текста подсказски используется он, в противном случае используется текст по умолчанию 
	КОНЕЦ ИНИЦИАЛИЗАЦИИ.
	По поводу всплывающих подсказок для кнопки IDC_ENTERPASSWORDDLG_SHOW_PASSWORD, на протяжении всей работы, если подсказски включены, точнее если диалоговое окно успешно создало окно подсказски(под успешно создало понимается что её дескриптор не равен nullptr),
	массив с текстами подсказок всегда будет провверяться на то, существует ли там строка, если она существует, то для подсказски юудет использоваться она
*/
/*UNICODE DlgProc`s versions*/
BDLDLL_API INT_PTR CALLBACK BDL::EnterPasswordDlgProcW(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {//функция обработки сообщений для диалога IDDBDL_ENTERPASSWORD
	HWND hEditPassword = GetDlgItem(hDlg, IDC_ENTERPASSWORDDLG_EDIT_PASSWORD), hShowPassword = GetDlgItem(hDlg, IDC_ENTERPASSWORDDLG_SHOW_PASSWORD), hClear = GetDlgItem(hDlg, IDC_DBL_ENTERPASSWORDCLEAR);
	static bool ShowPassword = true;//кнопка IDC_ENTERPASSWORDDLG_SHOW_PASSWORD выполняет показ пароля если true, в противном случае кнопка выполняет скрытие пароля
	static bool ClearButtonDisabled = true;//отключена ли кнопка "Очистить"
	static WCHAR PasswordChar = '\0';//Символ, отображаемый вместо пароля
	static HWND hToolTipForShowPassword = nullptr;//дексриптор  всплывающей подсказски
	static BDL::PEnterPasswordDlgInitParamW EPDI = nullptr;//указать на инициализационную структуру (строки в кодировке Unicode)
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
			if (!CompareTwoLines(EPDI->Caption, L"")){
				DefDlgProcW(hDlg, WM_SETTEXT, NULL, (LPARAM)EPDI->Caption);
			}
			if (!CompareTwoLines(EPDI->EditPasswordCaption, L"")) {
				SendMessageW(hEditPassword, EM_SETCUEBANNER, TRUE, (LPARAM)EPDI->EditPasswordCaption);
			}
			if (!CompareTwoLines(EPDI->OkButtonCaption, L"")) {
				HWND hOkButton = GetDlgItem(hDlg, IDOK);
				SetWindowTextW(hOkButton, EPDI->OkButtonCaption);
			}
			if (!CompareTwoLines(EPDI->CancelButtonCaption, L"")) {
				HWND hCancelButton = GetDlgItem(hDlg, IDCANCEL);
				SetWindowTextW(hCancelButton, EPDI->CancelButtonCaption);
			}
			if (!CompareTwoLines(EPDI->ClearButtonCaption, L"")) {
				HWND hClearButton = GetDlgItem(hDlg, IDC_DBL_ENTERPASSWORDCLEAR);
				SetWindowTextW(hClearButton, EPDI->ClearButtonCaption);
			}
			HICON hShowPasswordIcon = LoadIconW(hThisDll, MAKEINTRESOURCEW(IDI_EYE_VISIBLE));
			SendMessageW(hShowPassword, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hShowPasswordIcon);
			if (EPDI->HasToolTip == TRUE) {
				if (!CompareTwoLines(EPDI->ToolTipCaption[0], L""))hToolTipForShowPassword = BDL::CreateToolTipW(hShowPassword, hDlg, EPDI->ToolTipCaption[0], hThisDll);
				else {
					hToolTipForShowPassword = BDL::CreateToolTipW(hShowPassword, hDlg, L"Показать пароль", hThisDll);
				}
			}
			SetFocus(hEditPassword);
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
					if (textlength > 0) {
						*EPDI->Password = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR)*textlength);
						EPDI->PasswordSize = textlength;
						GetWindowTextW(hEditPassword, *EPDI->Password, textlength);
					}
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
							TI.lpszText = (!CompareTwoLines(EPDI->ToolTipCaption[1], L"") ? (LPWSTR)EPDI->ToolTipCaption[1] : (LPWSTR)L"Скрыть пароль");
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
							TI.lpszText = (!CompareTwoLines(EPDI->ToolTipCaption[0], L"") ? (LPWSTR)EPDI->ToolTipCaption[0] : (LPWSTR)L"Показать пароль");
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

bool BDL::CompareTwoLines(LPCWSTR str1, LPCWSTR str2) {
	if (str1 == nullptr) {
		if (str2 != nullptr) {
			return BDL::CompareTwoLines(str2, L"");
		}
		else return false;
	}
	else if (str2 == nullptr) {
		if (str1 != nullptr) {
			return BDL::CompareTwoLines(str1, L"");
		}
		else return false;
	}
	else {
		return (wcscmp(str1, str2) == 0);
	}
}
//данная функция конвертирует ANSI строку в Unicode строку
//Unicode версии вспомогательных функций
HWND BDL::CreateToolTipW(HWND hDlgItem, HWND hDlg, LPCWSTR pszText, HINSTANCE hInst) {
	if (!hDlgItem || !hDlg || !pszText) {
		return FALSE;
	}
	// Get the window of the tool.
	// Create the tooltip. g_hInst is the global instance handle.
	HWND hwndTip = CreateWindowExW(NULL, TOOLTIPS_CLASSW, NULL, WS_POPUP | TTS_ALWAYSTIP | TTS_BALLOON, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, hDlg, NULL, hInst, NULL);
	if (!hDlgItem || !hwndTip)return (HWND)NULL;
	// Associate the tooltip with the tool.
	TOOLINFOW toolInfo = { 0 };
	toolInfo.cbSize = sizeof(toolInfo);
	toolInfo.hwnd = hDlg;
	toolInfo.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
	toolInfo.uId = (UINT_PTR)hDlgItem;
	toolInfo.lpszText = (LPWSTR)pszText;
	SendMessageW(hwndTip, TTM_ADDTOOLW, 0, (LPARAM)&toolInfo);
	return hwndTip;
}
