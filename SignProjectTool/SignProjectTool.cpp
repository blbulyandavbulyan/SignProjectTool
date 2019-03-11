// SignProjectTool.cpp : Определяет точку входа для приложения.
//

#include "stdafx.h"
#include "SignProjectTool.h"

#define MAX_LOADSTRING 100

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна
struct ProgrammSettings {
	HINSTANCE hInst;
} PP;
// Отправить объявления функций, включенных в этот модуль кода:
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK StartConfigurationDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AddFilesForCertificationDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
WPARAM Absolute(LONG number);
UINT CalcBItemWidth(HWND hLB, LPTSTR Text);
void InitRegistryStorage();
void MessageError(TSTRING ErrorText, TSTRING ErrorCaption, HWND hWnd);
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
	PP.hInst = hInstance;
	//DialogBox(hInst, MAKEINTRESOURCE(IDD_START_CONFIGURATION), NULL, StartConfigurationDlgProc);
	DialogBox(hInst, MAKEINTRESOURCE(IDD_ADD_FILES_FOR_CERTIFICATION), NULL, AddFilesForCertificationDlgProc);
	return NULL;
}


//
//  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ЦЕЛЬ: Обрабатывает сообщения в главном окне.
//
//  WM_COMMAND  - обработать меню приложения
//  WM_PAINT    - Отрисовка главного окна
//  WM_DESTROY  - отправить сообщение о выходе и вернуться
//
//

// Обработчик сообщений для окна "О программе".
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
INT_PTR CALLBACK StartConfigurationDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	HWND hCertStoreName = GetDlgItem(hDlg, IDC_CERT_STORE_NAME), hOpenFile = GetDlgItem(hDlg, IDC_OPEN_FILE), hCertFile = GetDlgItem(hDlg, IDC_CERT_FILE), hCertStore = GetDlgItem(hDlg, IDC_CERT_STORE);
	static TCHAR szFile[MAX_PATH] = _TEXT("");// имя файла
	switch(message){
		case WM_INITDIALOG:{
			HICON hDialogIcon = LoadIcon(PP.hInst, MAKEINTRESOURCE(IDI_SIGNPROJECTTOOL));
			SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hDialogIcon);
			SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hDialogIcon);
			SendMessage(hCertStore, BM_SETCHECK, (WPARAM)BST_CHECKED, NULL);
			EnableWindow(hOpenFile, FALSE);
			return (INT_PTR)TRUE;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					if (_tcscmp(szFile, _TEXT("")) == 0) {
						MessageBox(hDlg, _TEXT("Вы не выбрали файл!"), _TEXT("Ошибка!"), MB_ICONERROR | MB_OK);
						break;
					}
					EndDialog(hDlg, IDCANCEL);
					return (INT_PTR)TRUE;
				case IDCANCEL:
					EndDialog(hDlg, IDCANCEL);
					return (INT_PTR)TRUE;
				case IDC_CERT_STORE:
					if (IsDlgButtonChecked(hDlg, IDC_CERT_STORE) == BST_CHECKED) {
						EnableWindow(hOpenFile, FALSE);
						EnableWindow(hCertStoreName, TRUE);
					}
					break;
				case IDC_CERT_FILE:
					if (IsDlgButtonChecked(hDlg, IDC_CERT_FILE) == BST_CHECKED) {
						EnableWindow(hOpenFile, TRUE);
						EnableWindow(hCertStoreName, FALSE);
					}
					break;
				case IDC_OPEN_FILE:{
					OPENFILENAME ofn;       // структура станд. блока диалога
					HANDLE hf;              // дескриптор файла
					// Инициализация структуры OPENFILENAME
					ZeroMemory(&ofn, sizeof(OPENFILENAME));
					ofn.hInstance = PP.hInst;
					ofn.lStructSize = sizeof(OPENFILENAME);
					ofn.hwndOwner = hDlg;
					ofn.lpstrFile = szFile;
					//ofn.lpstrFile[0] = '\0';
					ofn.nMaxFile = sizeof(szFile);
					ofn.lpstrFilter = _TEXT("pfx файлы(.pfx)\0*.pfx\0\0");
					ofn.nFilterIndex = 0;
					ofn.lpstrFileTitle = NULL;
					ofn.nMaxFileTitle = 0;
					ofn.lpstrInitialDir = NULL;
					ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
					// Показываем на экране диалоговое окно Открыть (Open).
					GetOpenFileName(&ofn);
					break;
				}
				
			}
			break;
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK AddFilesForCertificationDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	HWND hListSelectedFilesForCertification = GetDlgItem(hDlg, IDC_LIST_SELECTED_FILES_FOR_CERTIFICTION);
	static UINT MaxStringWhidth = 0;
	static DWORD MaxFileNameSize = MAX_PATH;
	switch (message) {
		case WM_INITDIALOG:{
			HICON hDialogIcon = LoadIcon(PP.hInst, MAKEINTRESOURCE(IDI_SIGNPROJECTTOOL));
			SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hDialogIcon);
			SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hDialogIcon);
			return (INT_PTR)TRUE;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					EndDialog(hDlg, IDOK);
					return (INT_PTR)TRUE;
				case IDCANCEL:
					EndDialog(hDlg, IDOK);
					return (INT_PTR)TRUE;
				case IDC_ADD_FILE:{
					TCHARVECTOR szFile;
					szFile.resize(MaxFileNameSize, _TEXT('\0'));
					OPENFILENAME ofn;       // структура станд. блока диалога
					HANDLE hf;              // дескриптор файла
					// Инициализация структуры OPENFILENAME
					ZeroMemory(&ofn, sizeof(OPENFILENAME));
					ofn.hInstance = PP.hInst;
					ofn.lStructSize = sizeof(OPENFILENAME);
					ofn.hwndOwner = hDlg;
					ofn.lpstrFile = szFile.data();
					//ofn.lpstrFile[0] = '\0';
					ofn.nMaxFile = szFile.size();
					ofn.lpstrFilter = _TEXT("Исполняемые файлы(.exe, .dll, .lib)\0*.exe;*.dll;*.lib\0\0");
					ofn.nFilterIndex = 0;
					ofn.lpstrFileTitle = NULL;
					ofn.nMaxFileTitle = 0;
					ofn.lpstrInitialDir = NULL;
					ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
					// Показываем на экране диалоговое окно Открыть (Open).
					OPENFILE://метка предназначена для повторного показа диалогового окна выбора файла, в случае если имя файла превышает MaxFileNameSize
					if (GetOpenFileName(&ofn) != NULL) {
						if (SendMessage(hListSelectedFilesForCertification, LB_ADDSTRING, NULL, (WPARAM)szFile.data()) != LB_ERR) {
							UINT Temp = CalcBItemWidth(hListSelectedFilesForCertification, szFile.data());
							if (Temp > MaxStringWhidth) {
								MaxStringWhidth = Temp;
								SendMessage(hListSelectedFilesForCertification, LB_SETHORIZONTALEXTENT, MaxStringWhidth, 0);
							}
						}
						else {
							MessageError(_TEXT("Не удалось добавить файл!"), _TEXT("Ошибка добавления файла!"), hDlg);
						}
						
					}
					else {
						switch (CommDlgExtendedError()) {
							case FNERR_BUFFERTOOSMALL:{
								MessageBox(hDlg, _TEXT("Так как полный путь к вашему файлу слишком велик и не помещается в буфер, мы увеличим размер буфера и вы выберете свой файл ещё раз"), _TEXT("Полный путь к файлу слишком велик"), MB_ICONINFORMATION | MB_OK);
								byte *MyArrayBytes = (byte *)szFile.data();
								memcpy_s(&ofn.nMaxFile, sizeof(ofn.nMaxFile), MyArrayBytes, 2 * sizeof(byte));
								szFile.resize(ofn.nMaxFile, _TEXT('\0'));
								ofn.lpstrFile = szFile.data();
								MaxFileNameSize = ofn.nMaxFile;
								goto OPENFILE;
								break;
							}
							case FNERR_INVALIDFILENAME:
								break;
							case FNERR_SUBCLASSFAILURE:
								break;
						}
					}
					break;
				}
				case IDC_DELETE_FILE:

					break;

			}
	}
	return (INT_PTR)FALSE;
}

WPARAM Absolute(LONG number) {
	return (number < 0 ? -number : number);
}

UINT CalcBItemWidth(HWND hLB, LPTSTR Text) {
	RECT r;
	HDC hLBDC = GetDC(hLB);
	HDC hDC = CreateCompatibleDC(hLBDC);
	HFONT hFont = (HFONT)SendMessage(hLB, WM_GETFONT, 0, 0);
	HGDIOBJ hOrgFont = SelectObject(hDC, hFont);
	ZeroMemory(&r, sizeof(r));
	DrawText(hDC, Text, -1, &r, DT_CALCRECT | DT_SINGLELINE | DT_NOCLIP);
	SelectObject(hDC, hOrgFont);
	DeleteDC(hDC);
	ReleaseDC(hLB, hLBDC);
	return (r.right - r.left) + (2 * GetSystemMetrics(SM_CXEDGE));
}

void InitRegistryStorage() {
}
void MessageError(TSTRING ErrorText, TSTRING ErrorCaption, HWND hWnd) {
	DWORD RHKError = GetLastError();
	LPTSTR BufferForFormatMessage = nullptr;
	DWORD FMResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, RHKError, LANG_SYSTEM_DEFAULT, (LPTSTR)&BufferForFormatMessage, NULL, nullptr);
	if (FMResult == 0) {
		MessageBox(hWnd, ErrorText.c_str(), ErrorCaption.c_str(), MB_OK | MB_ICONERROR);
		MessageBox(hWnd, _TEXT("Не удалось узнать возникновения предыдущей ошибки!"), _TEXT("Мне не удалось причину возникновения ошибки!"), MB_OK | MB_ICONERROR);
	}
	else {
		MessageBox(hWnd, BufferForFormatMessage, ErrorCaption.c_str(), MB_OK | MB_ICONERROR);
	}
	if (LocalFree((HLOCAL)BufferForFormatMessage) != 0) {
		MessageBox(hWnd, _TEXT("Не удалось освободить буфуер при обработке предыдущей ошибки!"), _TEXT("Ошибка освобождения буфера!"), MB_OK | MB_ICONERROR);
	}
}
/*void CreateRegistryKey(TSTRING KeyName, HKEY hRootKey, ProgrammParameters *pp) {
	if (KeyName == _TEXT("Settings")) {
		DWORD lpwdDispositionFromSettingsKey = 0;
		HKEY hRootSettings = 0;
		LSTATUS CreateASettingsKeyStatus = RegCreateKeyEx(hRootKey, _TEXT("Settings"), NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hRootSettings, &lpwdDispositionFromSettingsKey);// создание ветки реестра с общими настройками
		if (CreateASettingsKeyStatus == ERROR_SUCCESS) {
			LSTATUS CreateIntervalOfClicks = RegSetValueEx(hRootSettings, _TEXT("IntervalOfClicks"), NULL, REG_DWORD, (BYTE *)&pp->GlobalMiliSecondsOfClicks, sizeof(pp->GlobalMiliSecondsOfClicks));// создания переменной в реестре для хранения интервала кликов
			if (CreateIntervalOfClicks != ERROR_SUCCESS) {
				LPTSTR BufferForFormatMessage = nullptr;
				DWORD FMResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, CreateIntervalOfClicks, LANG_SYSTEM_DEFAULT, (LPTSTR)&BufferForFormatMessage, NULL, nullptr);
				OutputDebugString(_TEXT("Не удалось создать параметр IntervalOfClicks в ветке реестра HKEY_CURRENT_USER\\SOFTWARE\\Blbulyan Software\\Autoclicker\\Settings\n"));
				if (FMResult == 0) {
					OutputDebugString(_TEXT("Мне не удалось причину возникновения ошибки!\n"));
				}
				else {
					OutputDebugString(BufferForFormatMessage);
				}
				if (LocalFree((HLOCAL)BufferForFormatMessage) != 0) {
					OutputDebugString(_TEXT("Не удалось освободить буфуер при обработке предыдущей ошибки!"));
				}
			}
			LSTATUS CreateMouseButtonForClick = RegSetValueEx(hRootSettings, _TEXT("MouseButtonForClick"), NULL, REG_EXPAND_SZ, (BYTE *)_TEXT("LBTN"), sizeof(_TEXT("LBTN")) / sizeof(TCHAR));// создание переменной в реестре для хранения информации о том какой кнопкой мыши кликать
			if (CreateMouseButtonForClick != ERROR_SUCCESS) {
				LPTSTR BufferForFormatMessage = nullptr;
				DWORD FMResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, CreateIntervalOfClicks, LANG_SYSTEM_DEFAULT, (LPTSTR)&BufferForFormatMessage, NULL, nullptr);
				OutputDebugString(_TEXT("Не удалось создать параметр MouseButtonForClick в ветке реестра HKEY_CURRENT_USER\\SOFTWARE\\Blbulyan Software\\Autoclicker\\Settings\n"));
				if (FMResult == 0) {
					OutputDebugString(_TEXT("Мне не удалось причину возникновения ошибки!\n"));
				}
				else {
					OutputDebugString(BufferForFormatMessage);
				}
				if (LocalFree((HLOCAL)BufferForFormatMessage) != 0) {
					OutputDebugString(_TEXT("Не удалось освободить буфуер при обработке предыдущей ошибки!"));
				}
			}
			LSTATUS WhenStartingTheProgramMinimizeTheWindowToTray = RegSetValueEx(hRootSettings, _TEXT("WhenStartingTheProgramMinimizeTheWindowToTray"), NULL, REG_DWORD, (BYTE *)0, sizeof(UINT));// создание переменной в реестре которая отвечает за то, нужно ли сворачивать программу в трей при запуске, если значение равно 0 то не нужно, если больше нуля, то нужно
			if (CreateMouseButtonForClick != ERROR_SUCCESS) {
				LPTSTR BufferForFormatMessage = nullptr;
				DWORD FMResult = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, CreateIntervalOfClicks, LANG_SYSTEM_DEFAULT, (LPTSTR)&BufferForFormatMessage, NULL, nullptr);
				OutputDebugString(_TEXT("Не удалось создать параметр MouseButtonForClick в ветке реестра HKEY_CURRENT_USER\\SOFTWARE\\Blbulyan Software\\Autoclicker\\Settings\n"));
				if (FMResult == 0) {
					OutputDebugString(_TEXT("Мне не удалось причину возникновения ошибки!\n"));
				}
				else {
					OutputDebugString(BufferForFormatMessage);
				}
				if (LocalFree((HLOCAL)BufferForFormatMessage) != 0) {
					OutputDebugString(_TEXT("Не удалось освободить буфуер при обработке предыдущей ошибки!"));
				}
			}
		}
	}
	else if (KeyName == _TEXT("HotKey Settings")) {
		DWORD lpwdDispositionFromHotKetSettingsKey = 0;
		HKEY hRootHotKeySettings = 0;
		LSTATUS CreateAHotKeySettingsKeyStatus = RegCreateKeyEx(hRootKey, _TEXT("HotKey Settings"), NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hRootHotKeySettings, &lpwdDispositionFromHotKetSettingsKey);// создание ветки реестра с настройками горячих клавиш
	}
}*/