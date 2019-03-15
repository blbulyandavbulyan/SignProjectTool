﻿// SignProjectTool.cpp : Определяет точку входа для приложения.
//

#include "stdafx.h"
#include "SignProjectTool.h"
#define MAX_LOADSTRING 100

// Глобальные переменные:
//HINSTANCE hInst;                                // текущий экземпляр
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
#ifndef _UNICODE
UINT CalcBItemWidth(HWND hLB, PWSTR Text);
#endif
void InitRegistryStorage();
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
	//Parse command line
	PP.hInst = hInstance;
	WSTRINGARRAY CommandArray = BreakAStringIntoAnArrayOfStringsByCharacter(lpCmdLine, L' ');
	size_t CommandArraySize = CommandArray.size();
	for (size_t i = 0; i < CommandArraySize; i++) {
		if (CommandArray[i] == L"--about") {
			DialogBox(PP.hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), NULL, About);
			return NULL;
		}
		else if (CommandArray[i] == L"--startup-config") {
			DialogBox(PP.hInst, MAKEINTRESOURCE(IDD_START_CONFIGURATION), NULL, StartConfigurationDlgProc);
		}
	}
	if(DialogBox(PP.hInst, MAKEINTRESOURCE(IDD_START_CONFIGURATION), NULL, StartConfigurationDlgProc) == IDOK)DialogBox(PP.hInst, MAKEINTRESOURCE(IDD_ADD_FILES_FOR_CERTIFICATION), NULL, AddFilesForCertificationDlgProc);
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
		case WM_INITDIALOG:{
			HICON hDialogIcon = LoadIcon(PP.hInst, MAKEINTRESOURCE(IDI_SIGNPROJECTTOOL));
			if (hDialogIcon == NULL)MessageError(_TEXT("Не удалось загрузить иконку для текущего диалога!"), _TEXT("Ошибка загрузки иконки!"), hDlg);
			else {
				SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hDialogIcon);
				SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hDialogIcon);
			}
			return (INT_PTR)TRUE;
		}
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
			if (hDialogIcon == NULL)MessageError(_TEXT("Не удалось загрузить иконку для текущего диалога!"), _TEXT("Ошибка загрузки иконки!"), hDlg);
			else {
				SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hDialogIcon);
				SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hDialogIcon);
			}
			SendMessage(hCertStore, BM_SETCHECK, (WPARAM)BST_CHECKED, NULL);
			//начало операции удаления ненужного пункта меню
			HMENU hMainMenu = GetMenu(hDlg);//получения меню диалогового окна
			if (hMainMenu != NULL) {//если прошло успешно
				if (RemoveMenu(hMainMenu, IDM_STARTUPCONFIG, MF_BYCOMMAND) == NULL) {//то удаляем, если удаление завершилось неудачей
					MessageError(_TEXT("Не удалось удалить пункт меню!"), _TEXT("Ошибка удаления пункта меню!"), NULL);//выводим соотвествующую ошибку
					//начало процесса отключения пункта меню
					MENUITEMINFO mi;//струтура с информацией о пунтке меню
					ZeroMemory(&mi, sizeof(MENUITEMINFO));//обнуляем струткуру
					mi.cbSize = sizeof(MENUITEMINFO);//размер данной струтуры
					mi.fState = MFS_DISABLED;//свойство для отключения пунтка меню
					mi.fMask = MIIM_STATE;//чтобы действовала группа свойств (в которой находится MFS_DISABLED)
					if (SetMenuItemInfo(hMainMenu, IDM_STARTUPCONFIG, FALSE, &mi) == NULL)MessageError(_TEXT("Не удалось установить информацию о пункте меню!"), _TEXT("Ошибка установки информации о пункте меню!"), NULL);//отключаем, если неудача, то выводим ошибку
				}
			}
			else MessageError(_TEXT("Не удалось получить меню диалога!"), _TEXT("Ошибка получения меню диалога!"), NULL);
			EnableWindow(hOpenFile, FALSE);
			return (INT_PTR)TRUE;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDM_ABOUT:
					DialogBox(PP.hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hDlg, About);
					break;
				case IDM_EXIT:
					EndDialog(hDlg, IDM_EXIT);
					break;
				case IDOK:
					//if (_tcscmp(szFile, _TEXT("")) == 0) {
						//MessageBox(hDlg, _TEXT("Вы не выбрали файл!"), _TEXT("Ошибка!"), MB_ICONERROR | MB_OK);
						//break;
					//}
					EndDialog(hDlg, IDOK);
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
					
					break;
				}
				
			}
			break;
	}
	return (INT_PTR)FALSE;
}
// Функция обработки сообщений диалога IDD_ADD_FILES_FOR_CERTIFICATION
INT_PTR CALLBACK AddFilesForCertificationDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	HWND hListSelectedFilesForCertification = GetDlgItem(hDlg, IDC_LIST_SELECTED_FILES_FOR_CERTIFICTION), hDeleteFile = GetDlgItem(hDlg, IDC_DELETE_FILE), hModifyFile = GetDlgItem(hDlg, IDC_MODIFY_FILE), hSign = GetDlgItem(hDlg, IDC_SIGN), hClear = GetDlgItem(hDlg, IDC_CLEAR);
	static UINT MaxStringWhidth = 0;//переменная характеризует максимально возможное значение, на которое можно прокрутить горизонтальный скролбар ListBox
	switch (message) {
		case WM_INITDIALOG:{
			HICON hDialogIcon = LoadIcon(PP.hInst, MAKEINTRESOURCE(IDI_SIGNPROJECTTOOL));//загрузка иконки диалога из ресурсов
			if (hDialogIcon == NULL)MessageError(_TEXT("Не удалось загрузить иконку для текущего диалога!"), _TEXT("Ошибка загрузки иконки!"), hDlg);
			else {
				//установка иконки диалога
				SendMessage(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hDialogIcon);
				SendMessage(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hDialogIcon);
			}
			return (INT_PTR)TRUE;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDM_STARTUPCONFIG:
					DialogBox(PP.hInst, MAKEINTRESOURCE(IDD_START_CONFIGURATION), NULL, StartConfigurationDlgProc);
					break;
				case IDM_ABOUT: //обработка пункта меню "О программе"
					DialogBox(PP.hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hDlg, About);
					break;
				case IDC_SIGN://обработка кнопки "Подписать"
					EndDialog(hDlg, IDC_SIGN);
					return (INT_PTR)TRUE;
				case IDM_EXIT://обработка пункта меню "Выход"
				case IDCANCEL://обработка кнопки "Выход"
					EndDialog(hDlg, IDOK);
					return (INT_PTR)TRUE;
				case IDC_ADD_FILE:{// обработка кнопки "Добавить"
					HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
						COINIT_DISABLE_OLE1DDE);
					if (SUCCEEDED(hr)) {
						IFileOpenDialog *pFileOpen;
						// Create the FileOpenDialog object.
						hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
							IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));
						if (SUCCEEDED(hr)) {
							// Show the Open dialog box.
							COMDLG_FILTERSPEC cmf[5];
							//фильтр для *.exe файлов
							cmf[0].pszName = L"*.exe файлы";
							cmf[0].pszSpec = L"*.exe";
							//фильтр для *.dll файлов
							cmf[1].pszName = L"*.dll файлы";
							cmf[1].pszSpec = L"*.dll";
							//фильтр для *.lib файлов
							cmf[2].pszName = L"*.lib файлы";
							cmf[2].pszSpec = L"*.lib";
							//фильтр *.cab файлов
							cmf[3].pszName = L"*.cab файлы";
							cmf[3].pszSpec = L"*.cab";
							//фильтр для *.exe, *.dll, *.lib, *.cab файлов
							cmf[4].pszName = L"*.exe, *.dll, *.lib, *.cab файлы";
							cmf[4].pszSpec = L"*.exe;*.dll;*.lib;*.cab";
							hr = pFileOpen->SetFileTypes(sizeof(cmf) / sizeof(cmf[0]), cmf);
							if (SUCCEEDED(hr)){
								hr = pFileOpen->SetTitle(L"Пожалуйста, выберете файлы для цифровой подписи");
								if (hr != S_OK)MessageError(_TEXT("Не удалось установить заголовок для диалогового окна открытия файлов!"), _TEXT("Ошибка установки заголовка!"), hDlg, hr);
								hr = pFileOpen->SetOptions(FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST | FOS_ALLOWMULTISELECT);
								if (SUCCEEDED(hr)) {
									hr = pFileOpen->Show(hDlg);
									// Get the file name from the dialog box.
									if (SUCCEEDED(hr)) {
										IShellItemArray *pItem = nullptr;
										hr = pFileOpen->GetResults(&pItem);
										if (SUCCEEDED(hr)) {
											DWORD FilesCount = 0;
											hr = pItem->GetCount(&FilesCount);
											if (SUCCEEDED(hr)) {
												for (DWORD i = 0; i < FilesCount; i++) {
													IShellItem *MyFile = nullptr;
													hr = pItem->GetItemAt(i, &MyFile);
													if (SUCCEEDED(hr)) {
														PWSTR pszFilePath;
														hr = MyFile->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
														// Display the file name to the user.
														if (SUCCEEDED(hr)) {
															if (SendMessageW(hListSelectedFilesForCertification, LB_ADDSTRING, NULL, (WPARAM)pszFilePath) != LB_ERR) {
																UINT Temp = CalcBItemWidth(hListSelectedFilesForCertification, pszFilePath);
																if (Temp > MaxStringWhidth)MaxStringWhidth = Temp;
															}
															else MessageError(_TEXT("Не удалось добавить файл!"), _TEXT("Ошибка добавления файла!"), hDlg);
															CoTaskMemFree(pszFilePath);
														}
														else MessageError(_TEXT("Не удалось получить имя открытого вами файла, файл не будет добавлен!"), _TEXT("Ошибка получения имени открытого файла!"), hDlg, hr);
														MyFile->Release();
													}
													else MessageError(_TEXT("Не удалось получить один из открытых вами файлов, файл не будет добавлен!"), _TEXT("Ошибка получения открытого файла!"), hDlg, hr);
												}
												if (FilesCount > 0) {
													SendMessage(hListSelectedFilesForCertification, LB_SETHORIZONTALEXTENT, MaxStringWhidth, 0);
													EnableWindow(hDeleteFile, TRUE);
													EnableWindow(hModifyFile, TRUE);
													EnableWindow(hSign, TRUE);
													EnableWindow(hClear, TRUE);
												}
											}
											else MessageError(_TEXT("Не удалось получить количество открытых вами файлов!"), _TEXT("Ошибка получения кол-ва открытых файлов!"), hDlg, hr);
											pItem->Release();
										}
										else MessageError(_TEXT("Не удалось получить открытые вами файлы!"), _TEXT("Ошибка получения открытых вами файлов!"), hDlg, hr);
									}
									else MessageError(_TEXT("Не удалось показать диалоговое окно открытия файлов, дальнейшее открытие файлов невозможно!"), _TEXT("Ошибка показа диалогового окна!"), hDlg, hr);
									pFileOpen->Release();
								}
								else MessageError(_TEXT("Не удалось установить необходимые опции для файлового диалога, дальнейшее открытие файлов невозможно!"), _TEXT("Ощибка установки опций!"), hDlg, hr);
							}
							else MessageError(_TEXT("Не удалось установить фильтры допустимых файлов, невозможно добавить файлы!"), _TEXT("Ошибка установки фильтров для файлового диалога!"), hDlg, hr);
						}
						else MessageError(_TEXT("Не удалось создать объект CLSID_FileOpenDialog, дальнейшее открытие файлов невозможно!"), _TEXT("Ошибка создания объекта CLSID_FileOpenDialog!"), hDlg, hr);
						CoUninitialize();
					}
					else MessageError(_TEXT("Не удалось инициализировать библиотеку COM!"), _TEXT("Ошибка инициализиции библиотеки COM!"), hDlg, hr);
				}
				case IDC_DELETE_FILE:{//обработки кнопки "Удалить"
					LRESULT ItemsCount = SendMessage(hListSelectedFilesForCertification, LB_GETCOUNT, NULL, NULL);// получение кол-ва элементов(файлов) в списке
					if (ItemsCount != LB_ERR) {//проверка на ошибку
						MaxStringWhidth = 0;//обнуление длинны прокрутки горизонтального скролбара ListBox
						for (UINT i = 0; i < ItemsCount; i++) {//цикл перебора строк 
							LRESULT ItemIsSelected = SendMessage(hListSelectedFilesForCertification, LB_GETSEL, (WPARAM)i, NULL);//получение информации о том, выбран ли элемент
							if (ItemIsSelected > 0) {// если выбран
								if (SendMessage(hListSelectedFilesForCertification, LB_DELETESTRING, (WPARAM)i, NULL) != LB_ERR) {// и если не ошибка, то удаляем его
									ItemsCount--; i--;// и уменьшаем итератор и кол-во элементов в списке
								}
								else MessageError(_TEXT("Не удалось удалить выбранный элемент из списка файлов!"), _TEXT("Ошибка удаления выбранного элемента!"), hDlg);// в случае ошибки
							}
							else if (ItemIsSelected == 0) {// если не выбран
								TCHARVECTOR ListBoxString;// здесь будет хранится полученная строка из ListBox
								LRESULT TextLen = SendMessage(hListSelectedFilesForCertification, LB_GETTEXTLEN, (WPARAM)i, NULL);//получаем длинну строки
								if (TextLen != LB_ERR) {// если не ошибка
									ListBoxString.resize(TextLen+1);//инициализируем массив ListBoxString, к TextLen прибавляем 1, т.к. длинна, которую мы получили в предыдущем шаге не включает терминирующий ноль
									if (SendMessage(hListSelectedFilesForCertification, LB_GETTEXT, (WPARAM)i, (LPARAM)ListBoxString.data()) != LB_ERR) {//получаем текст элемента, если он был получен успешно
										UINT Temp = CalcBItemWidth(hListSelectedFilesForCertification, ListBoxString.data());//пересчитываем длинну прокрутки горизонтального скролбара
										if (Temp > MaxStringWhidth) MaxStringWhidth = Temp;//если она больше максимальной, то присваиваем её максимальной
									}
									else MessageError(_TEXT("Не удалось получить текст невыбранного элемента!"), _TEXT("Ошибка при получении текста невыбранного элемента!"), hDlg);//в случае неудачного получения текста элемента
								}
								else MessageError(_TEXT("Не удалось узнать длинну текста невыбранного элемента!"), _TEXT("Ошибка получения длинны невыбранного элемента!"), hDlg);//в случае неудачного получения длинны строки
							}
							else if (ItemIsSelected == LB_ERR)MessageError(_TEXT("Не удалось узнать выбран ли элемент"), _TEXT("Ошибка при распознавании выбранного элемента!"), hDlg);//в случае ошибки проверки выбран ли элемент
						}
						if (ItemsCount == 0) {// если список стал пустым
							//отключение всех органов управления, для которых требуется наличие хотя бы одного файла в списке
							EnableWindow(hDeleteFile, FALSE);
							EnableWindow(hModifyFile, FALSE);
							EnableWindow(hSign, FALSE);
							EnableWindow(hClear, FALSE);
						}
						if (SendMessage(hListSelectedFilesForCertification, LB_SETHORIZONTALEXTENT, MaxStringWhidth, 0) == LB_ERR)MessageError(_TEXT("Не удалось установить прокручиваемую область!"), _TEXT("Ошибка установки прокручиваемой области!"), hDlg);//реинициализируем горизонтальный скролбар ListBox новым значением длинны прокрутки
					}
					else MessageError(_TEXT("Не удалось получить количество элементов"), _TEXT("Ошибка получения количества элементов!"), hDlg);//в случае ошибки получения кол-ва элементов
					break;
				} 
				case IDC_MODIFY_FILE: //обработка кнопки "Изменить"
					
					break;
				case IDC_CLEAR://обработка кнопки "Очистить
					if (SendMessage(hListSelectedFilesForCertification, LB_RESETCONTENT, NULL, NULL) == LB_ERR)MessageError(_TEXT("Не удалось очистить список!"), _TEXT("Ошибка очистки списка!"), hDlg);
					MaxStringWhidth = 0;//обнуление длинны прокрутки горизонтального скролбара ListBox
					// "выключение" горизонтального скролбара
					if (SendMessage(hListSelectedFilesForCertification, LB_SETHORIZONTALEXTENT, MaxStringWhidth, 0) == LB_ERR)MessageError(_TEXT("Не удалось установить прокручиваемую область!"), _TEXT("Ошибка установки прокручиваемой области!"), hDlg);
					//отключение всех органов управления, для которых требуется наличие хотя бы одного файла в списке
					EnableWindow(hDeleteFile, FALSE);
					EnableWindow(hModifyFile, FALSE);
					EnableWindow(hSign, FALSE);
					EnableWindow(hClear, FALSE);
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
#ifndef _UNICODE
UINT CalcBItemWidth(HWND hLB, PWSTR Text) {
	RECT r;
	HDC hLBDC = GetDC(hLB);
	HDC hDC = CreateCompatibleDC(hLBDC);
	HFONT hFont = (HFONT)SendMessageW(hLB, WM_GETFONT, 0, 0);
	HGDIOBJ hOrgFont = SelectObject(hDC, hFont);
	ZeroMemory(&r, sizeof(r));
	DrawTextW(hDC, Text, -1, &r, DT_CALCRECT | DT_SINGLELINE | DT_NOCLIP);
	SelectObject(hDC, hOrgFont);
	DeleteDC(hDC);
	ReleaseDC(hLB, hLBDC);
	return (r.right - r.left) + (2 * GetSystemMetrics(SM_CXEDGE));
}
#endif
void InitRegistryStorage() {
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