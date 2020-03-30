#include "stdafx.h"
extern ProgrammStatement PS;
//процедура обработки сообщений диалогового окна настройки TimeStamp серверов IDD_SETTINGS_TIMESTAMP_SERVERS
INT_PTR CALLBACK SettingsTimeStampsServersDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	HWND hClearInput = GetDlgItem(hDlg, IDC_CLEAR_INPUT), hAddTimeStampServer = GetDlgItem(hDlg, IDC_ADD_TIMESTAMP_SERVER), hDeleteTimeStampServer = GetDlgItem(hDlg, IDC_DELETE_TIMESTAMP_SERVER), hClearTimeStampServers = GetDlgItem(hDlg, IDC_CLEAR_TIMESTAMP_SERVERS), hListTimeStampServers = GetDlgItem(hDlg, IDC_LIST_TIMESTAMP_SERVERS), hEditTimeStampServer = GetDlgItem(hDlg, IDC_EDIT_TIMESTAMP_SERVER), hItemsCount = GetDlgItem(hDlg, IDC_ITEMS_COUNT);
	static UINT MaxStringWhidth = 0;//переменная характеризует максимально возможное значение, на которое можно прокрутить горизонтальный скролбар ListBox
	static WSTRINGARRAY* TimeStampList = nullptr;
	switch (message) {
	case WM_INITDIALOG: {
		HICON hDialogIcon = LoadIconW(PS.hInst, MAKEINTRESOURCEW(IDI_SIGNPROJECTTOOL));
		if (hDialogIcon == NULL)WWS::MessageError(L"Не удалось загрузить иконку для текущего диалога!", L"Ошибка загрузки иконки!", hDlg);
		else {
			SendMessageW(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hDialogIcon);
			SendMessageW(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hDialogIcon);
		}
		TimeStampList = (WSTRINGARRAY*)lParam;
		return (INT_PTR)TRUE;
	}
	case WM_COMMAND:
		switch (HIWORD(wParam)) {
			case EN_CHANGE:
				switch (LOWORD(wParam)) {
				case IDC_EDIT_TIMESTAMP_SERVER: {
					if (GetWindowTextLengthW((HWND)lParam) > 0) {
						if (!IsWindowEnabled(hClearInput))EnableWindow(hClearInput, TRUE);
						if (!IsWindowEnabled(hAddTimeStampServer))EnableWindow(hAddTimeStampServer, TRUE);
					}
					else {
						EnableWindow(hClearInput, FALSE);
						EnableWindow(hAddTimeStampServer, FALSE);
					}
					break;
				}
											  break;
				}
				break;
			case LBN_SELCHANGE: {
				LRESULT ItemsCountSelected = SendMessageW(hListTimeStampServers, LB_GETSELCOUNT, NULL, NULL);// получение кол-ва выбранных элементов(файлов) в списке
				if (ItemsCountSelected != LB_ERR) {
					if (ItemsCountSelected > 0)EnableWindow(hDeleteTimeStampServer, TRUE);
					else EnableWindow(hDeleteTimeStampServer, FALSE);
				}
				else WWS::MessageError(L"Не удалось получить количество выбранных элементов", L"Ошибка получения количества элементов!", hDlg);//в случае ошибки получения кол-ва элементов
				break;
			}
			}
			switch (LOWORD(wParam)) {
			case IDOK: {
				LRESULT ItemsCount = SendMessageW(hListTimeStampServers, LB_GETCOUNT, NULL, NULL);// получение кол-ва элементов(файлов) в списке
				if (ItemsCount != LB_ERR) {
					for (LRESULT i = 0; i < ItemsCount; i++) {
						LRESULT TextLen = SendMessageW(hListTimeStampServers, LB_GETTEXTLEN, (WPARAM)i, NULL);//получаем длинну строки
						if (TextLen != LB_ERR) {// если не ошибка
							WCHARVECTOR ListBoxString;// здесь будет хранится полученная строка из ListBox
							ListBoxString.resize(TextLen + 1);//инициализируем массив ListBoxString, к TextLen прибавляем 1, т.к. длинна, которую мы получили в предыдущем шаге не включает терминирующий ноль
							if (SendMessageW(hListTimeStampServers, LB_GETTEXT, (WPARAM)i, (LPARAM)ListBoxString.data()) != LB_ERR) {//получаем текст элемента, если он был получен успешно
								TimeStampList->push_back(ListBoxString.data());
							}
							else WWS::MessageError(L"Не удалось получить текст невыбранного элемента!", L"Ошибка при получении текста невыбранного элемента!", hDlg);//в случае неудачного получения текста элемента
						}
						else WWS::MessageError(L"Не удалось узнать длинну текста невыбранного элемента!", L"Ошибка получения длинны невыбранного элемента!", hDlg);//в случае неудачного получения длинны строки
					}
				}
				else WWS::MessageError(L"Не удалось получить количество элементов", L"Ошибка получения количества элементов!", hDlg);//в случае ошибки получения кол-ва элементов
			}
			case IDCANCEL:
				EndDialog(hDlg, LOWORD(wParam));
				break;
			case IDC_DELETE_TIMESTAMP_SERVER: {
				LRESULT ItemsCount = SendMessageW(hListTimeStampServers, LB_GETCOUNT, NULL, NULL);// получение кол-ва элементов(файлов) в списке
				if (ItemsCount != LB_ERR) {//проверка на ошибку
					MaxStringWhidth = 0;//обнуление длинны прокрутки горизонтального скролбара ListBox
					for (LRESULT i = 0; i < ItemsCount; i++) {//цикл перебора строк 
						LRESULT ItemIsSelected = SendMessageW(hListTimeStampServers, LB_GETSEL, (WPARAM)i, NULL);//получение информации о том, выбран ли элемент
						if (ItemIsSelected > 0) {// если выбран
							if (SendMessageW(hListTimeStampServers, LB_DELETESTRING, (WPARAM)i, NULL) != LB_ERR) {// и если не ошибка, то удаляем его
								ItemsCount--; i--;// и уменьшаем итератор и кол-во элементов в списке
							}
							else WWS::MessageError(L"Не удалось удалить выбранный элемент из списка файлов!", L"Ошибка удаления выбранного элемента!", hDlg);// в случае ошибки
						}
						else if (ItemIsSelected == 0) {// если не выбран
							WCHARVECTOR ListBoxString;// здесь будет хранится полученная строка из ListBox
							LRESULT TextLen = SendMessageW(hListTimeStampServers, LB_GETTEXTLEN, (WPARAM)i, NULL);//получаем длинну строки
							if (TextLen != LB_ERR) {// если не ошибка
								ListBoxString.resize(TextLen + 1);//инициализируем массив ListBoxString, к TextLen прибавляем 1, т.к. длинна, которую мы получили в предыдущем шаге не включает терминирующий ноль
								if (SendMessageW(hListTimeStampServers, LB_GETTEXT, (WPARAM)i, (LPARAM)ListBoxString.data()) != LB_ERR) {//получаем текст элемента, если он был получен успешно
									UINT Temp = CalcBItemWidth(hListTimeStampServers, ListBoxString.data());//пересчитываем длинну прокрутки горизонтального скролбара
									if (Temp > MaxStringWhidth) MaxStringWhidth = Temp;//если она больше максимальной, то присваиваем её максимальной
								}
								else WWS::MessageError(L"Не удалось получить текст невыбранного элемента!", L"Ошибка при получении текста невыбранного элемента!", hDlg);//в случае неудачного получения текста элемента
							}
							else WWS::MessageError(L"Не удалось узнать длинну текста невыбранного элемента!", L"Ошибка получения длинны невыбранного элемента!", hDlg);//в случае неудачного получения длинны строки
						}
						else if (ItemIsSelected == LB_ERR)WWS::MessageError(L"Не удалось узнать выбран ли элемент", L"Ошибка при распознавании выбранного элемента!", hDlg);//в случае ошибки проверки выбран ли элемент
					}
					if (ItemsCount == 0) {// если список стал пустым
						//отключение всех органов управления, для которых требуется наличие хотя бы одного файла в списке
						EnableWindow(hDeleteTimeStampServer, FALSE);
						EnableWindow(hClearTimeStampServers, FALSE);
					}
					WSTRING ItemsCountText = L"Список TimeStamp серверов(" + std::to_wstring(ItemsCount) + L"):";
					SetWindowTextW(hItemsCount, ItemsCountText.c_str());
					if (SendMessageW(hListTimeStampServers, LB_SETHORIZONTALEXTENT, MaxStringWhidth, 0) == LB_ERR)WWS::MessageError(L"Не удалось установить прокручиваемую область!", L"Ошибка установки прокручиваемой области!", hDlg);//реинициализируем горизонтальный скролбар ListBox новым значением длинны прокрутки
				}
				else WWS::MessageError(L"Не удалось получить количество элементов", L"Ошибка получения количества элементов!", hDlg);//в случае ошибки получения кол-ва элементов
				break;
			}
			case IDC_CLEAR_INPUT:
				SetWindowTextW(hEditTimeStampServer, L"");
				break;
			case IDC_CLEAR_TIMESTAMP_SERVERS:
				if (SendMessageW(hListTimeStampServers, LB_RESETCONTENT, NULL, NULL) == LB_ERR)WWS::MessageError(L"Не удалось очистить список!", L"Ошибка очистки списка!", hDlg);
				MaxStringWhidth = 0;//обнуление длинны прокрутки горизонтального скролбара ListBox
				// "выключение" горизонтального скролбара
				if (SendMessageW(hListTimeStampServers, LB_SETHORIZONTALEXTENT, MaxStringWhidth, 0) == LB_ERR)WWS::MessageError(L"Не удалось установить прокручиваемую область!", L"Ошибка установки прокручиваемой области!", hDlg);
				EnableWindow(hDeleteTimeStampServer, FALSE);
				EnableWindow(hClearTimeStampServers, FALSE);
				SetWindowTextW(hItemsCount, L"Список TimeStamp серверов:");
				break;
			case IDC_ADD_TIMESTAMP_SERVER: {
				INT AddedTimeStampServerLength = GetWindowTextLengthW(hEditTimeStampServer);
				if (AddedTimeStampServerLength > 0) {
					WCHARVECTOR AddedTimeStampServer;
					AddedTimeStampServer.resize(AddedTimeStampServerLength + 1);
					if (GetWindowTextW(hEditTimeStampServer, AddedTimeStampServer.data(), AddedTimeStampServerLength + 1)) {
						if (!StringExistInListBox(hListTimeStampServers, AddedTimeStampServer.data())) {
							if (SendMessageW(hListTimeStampServers, LB_ADDSTRING, NULL, (LPARAM)AddedTimeStampServer.data()) != LB_ERR) {
								MaxStringWhidth = CalcBItemWidth(hListTimeStampServers, AddedTimeStampServer.data());
								if (SendMessageW(hListTimeStampServers, LB_SETHORIZONTALEXTENT, MaxStringWhidth, 0) == LB_ERR)WWS::MessageError(L"Не удалось установить прокручиваемую область!", L"Ошибка установки прокручиваемой области!", hDlg);//реинициализируем горизонтальный скролбар ListBox новым значением длинны прокрутки
								EnableWindow(hClearTimeStampServers, TRUE);
								SetWindowTextW(hEditTimeStampServer, L"");
								LRESULT ItemsCount = SendMessageW(hListTimeStampServers, LB_GETCOUNT, NULL, NULL);// получение кол-ва элементов(файлов) в списке
								if (ItemsCount != LB_ERR) {
									WSTRING ItemsCountText = L"Список TimeStamp серверов(" + std::to_wstring(ItemsCount) + L"):";
									SetWindowTextW(hItemsCount, ItemsCountText.c_str());
								}
							}
							else WWS::MessageError(L"Не удалось добавить TimeStamp сервер в список!", L"Ошибка добавления элемента в список!", hDlg);
						}
						else SetWindowTextW(hEditTimeStampServer, L"");
					}
				}
				break;
			}
		}
		break;
	}
	return (INT_PTR)FALSE;
}
INT_PTR CALLBACK SettingsForActionsPerformedAfterSigningDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	static WORD result = 0;
	switch (message) {
		case WM_INITDIALOG: {
			HICON hDialogIcon = LoadIconW(PS.hInst, MAKEINTRESOURCEW(IDI_SIGNPROJECTTOOL));
			if (hDialogIcon == NULL)WWS::MessageError(L"Не удалось загрузить иконку для текущего диалога!", L"Ошибка загрузки иконки!", hDlg);
			else {
				SendMessageW(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hDialogIcon);
				SendMessageW(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hDialogIcon);
			}
			result = *(PWORD)lParam;
			//начало инициализации радиокнопок
			if (result & ProgrammSettings::COMMAND_AFTER_SIGNING_FILES::CASF_DELETE_ALL_FILES_FROM_LIST)CheckDlgButton(hDlg, IDC_RADIO_DELETE_ALL_FILES_FROM_LIST, BST_CHECKED);
			else {
				if (result & ProgrammSettings::COMMAND_AFTER_SIGNING_FILES::CASF_DELETE_NOT_SUCCESSFULLY_SIGNED_FILES_FROM_LIST)CheckDlgButton(hDlg, IDC_RADIO_DELETE_NO_SIGNING_FILES_FROM_LIST, BST_CHECKED);
				if (result & ProgrammSettings::COMMAND_AFTER_SIGNING_FILES::CASF_DELETE_SUCCESSFULLY_SIGNED_FILES_FROM_LIST)CheckDlgButton(hDlg, IDC_RADIO_DELETE_SIGNING_FILES_FROM_LIST, BST_CHECKED);
			}

			return (INT_PTR)TRUE;
		}
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
			case IDOK:
				EndDialog(hDlg, MAKELONG(IDOK, result));
			case IDCANCEL:
				EndDialog(hDlg, MAKELONG(IDCANCEL, 0));
				break;
				/*
					0b00000000 - ничего не делать
					0b00000001 - удалить успешно подписанные файлы
					0b00000010 - удалить недаучноподписанные файлы
					0b00000011 - удалить все файлы

				*/
			case IDC_RADIO_DELETE_ALL_FILES_FROM_LIST:
				result |= ProgrammSettings::COMMAND_AFTER_SIGNING_FILES::CASF_DELETE_ALL_FILES_FROM_LIST;
				break;
			case IDC_RADIO_DELETE_NO_SIGNING_FILES_FROM_LIST:
				result &= ~ProgrammSettings::COMMAND_AFTER_SIGNING_FILES::CASF_DELETE_SUCCESSFULLY_SIGNED_FILES_FROM_LIST;
				result |= ProgrammSettings::COMMAND_AFTER_SIGNING_FILES::CASF_DELETE_NOT_SUCCESSFULLY_SIGNED_FILES_FROM_LIST;
				break;
			case IDC_RADIO_DELETE_SIGNING_FILES_FROM_LIST:
				result &= ~ProgrammSettings::COMMAND_AFTER_SIGNING_FILES::CASF_DELETE_NOT_SUCCESSFULLY_SIGNED_FILES_FROM_LIST;
				result |= ProgrammSettings::COMMAND_AFTER_SIGNING_FILES::CASF_DELETE_SUCCESSFULLY_SIGNED_FILES_FROM_LIST;
				break;
			case IDC_RADIO_NO_MODIFY_FILES_LIST:
				result &= ~ProgrammSettings::COMMAND_AFTER_SIGNING_FILES::CASF_DELETE_ALL_FILES_FROM_LIST;
				break;
			}
			break;
		}
	}
	return (INT_PTR)FALSE;
}
INT_PTR CALLBACK StartConfigurationDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	HWND hOpenFromCertStore = GetDlgItem(hDlg, IDC_BUTTON_SELECT_CERTIFICATE_FROM_STORE), hOpenFile = GetDlgItem(hDlg, IDC_OPEN_FILE), hCertFile = GetDlgItem(hDlg, IDC_CERT_FILE), hCertStore = GetDlgItem(hDlg, IDC_CERT_STORE), hCertList = GetDlgItem(hDlg, IDC_COMBOX_LISTCERT_FILES);
	//static WSTRING CertificateFileName;
	static UINT MaxHScrollWidth = NULL;
	static START_CONFIGURATION_RESULT LSCR;
	static START_CONFIGURATION_RESULT::PSTART_CONFIRURATION_RESULT PSCR = NULL;
	switch (message) {
		case WM_INITDIALOG: {
			PSCR = (START_CONFIGURATION_RESULT::PSTART_CONFIRURATION_RESULT)lParam;
			//CertificateFileName = PP.CertificateFile;
			HICON hDialogIcon = LoadIconW(PS.hInst, MAKEINTRESOURCEW(IDI_SIGNPROJECTTOOL));
			if (hDialogIcon == NULL)WWS::MessageError(L"Не удалось загрузить иконку для текущего диалога!", L"Ошибка загрузки иконки!", hDlg);
			else {
				SendMessageW(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hDialogIcon);
				SendMessageW(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hDialogIcon);
			}
			SendMessageW(hCertStore, BM_SETCHECK, (WPARAM)BST_CHECKED, NULL);
			//начало операции удаления ненужного пункта меню
			HMENU hMainMenu = GetMenu(hDlg);//получения меню диалогового окна
			if (hMainMenu != NULL) {//если прошло успешно
				if (RemoveMenu(hMainMenu, IDM_STARTUPCONFIG, MF_BYCOMMAND) == NULL) {//то удаляем, если удаление завершилось неудачей
					WWS::MessageError(L"Не удалось удалить пункт меню!", L"Ошибка удаления пункта меню!", NULL);//выводим соотвествующую ошибку
					//начало процесса отключения пункта меню
					MENUITEMINFOW mi;//струтура с информацией о пунтке меню
					ZeroMemory(&mi, sizeof(MENUITEMINFOW));//обнуляем струткуру
					mi.cbSize = sizeof(MENUITEMINFOW);//размер данной струтуры
					mi.fState = MFS_DISABLED;//свойство для отключения пунтка меню
					mi.fMask = MIIM_STATE;//чтобы действовала группа свойств (в которой находится MFS_DISABLED)
					if (SetMenuItemInfoW(hMainMenu, IDM_STARTUPCONFIG, FALSE, &mi) == NULL)WWS::MessageError(L"Не удалось установить информацию о пункте меню!", L"Ошибка установки информации о пункте меню!", NULL);//отключаем, если неудача, то выводим ошибку
				}
			}
			else WWS::MessageError(L"Не удалось получить меню диалога!", L"Ошибка получения меню диалога!", NULL);
			EnableWindow(hOpenFile, FALSE);
			return (INT_PTR)TRUE;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDM_ABOUT:
					DialogBoxW(PS.hInst, MAKEINTRESOURCEW(IDD_ABOUTBOX), hDlg, About);
					break;
				case IDM_EXIT:
					EndDialog(hDlg, IDM_EXIT);
					break;
				case IDOK:
					*PSCR = LSCR;
				case IDCANCEL:
					EndDialog(hDlg, LOWORD(wParam));
					return (INT_PTR)TRUE;
				case IDC_CERT_STORE:
					if (IsDlgButtonChecked(hDlg, IDC_CERT_STORE) == BST_CHECKED) {
						EnableWindow(hOpenFile, FALSE);
						EnableWindow(hCertList, FALSE);
						EnableWindow(hOpenFromCertStore, TRUE);
					}
					break;
				case IDC_CERT_FILE:
					if (IsDlgButtonChecked(hDlg, IDC_CERT_FILE) == BST_CHECKED) {
						EnableWindow(hOpenFile, TRUE);
						EnableWindow(hCertList, TRUE);
						EnableWindow(hOpenFromCertStore, FALSE);
					}
					break;
				case IDC_BUTTON_SELECT_CERTIFICATE_FROM_STORE: {
					HCERTSTORE hSysStore = CertOpenStore(CERT_STORE_PROV_SYSTEM_W, X509_ASN_ENCODING, NULL, CERT_SYSTEM_STORE_CURRENT_USER, L"My");
					if (hSysStore != NULL) {
						PCCERT_CONTEXT CertificateForSigning = NULL;

						if (CertificateForSigning != NULL) {
							CertFreeCertificateContext(CertificateForSigning);
						}
						CertCloseStore(hSysStore, CERT_CLOSE_STORE_FORCE_FLAG);
					}
					break;
				}
				case IDC_OPEN_CERT_FILE: {
					COMDLG_FILTERSPEC cmf[1] = {//массив с фильтрами
						//фильтр для *.pfx файлов
						{
							L"*.pfx файлы",
							L"*.pfx"
						}
					};
					WSTRINGARRAY CertificateFile = OpenFiles(hDlg, cmf, sizeof(cmf) / sizeof(COMDLG_FILTERSPEC), L"Загрузите сертификат для подписи", L"Загрузить", false);
					if (CertificateFile.size() > 0) {
						if (SendMessageW(hCertList, CB_SELECTSTRING, -1, (LPARAM)CertificateFile[0].c_str()) == CB_ERR) {
							LRESULT AddStringResult = SendMessageW(hCertList, CB_ADDSTRING, 0, (LPARAM)CertificateFile[0].c_str());
							if (AddStringResult == CB_ERR || AddStringResult == CB_ERRSPACE)WWS::MessageError(L"Ошибка добавления строки с именем файла сертификат с соответсвующий список!", L"Ошибка при добавлении стрроки в combobox", hDlg);
							//тут должен быть код включения горизонтальной прокрутки в раскрывающемся списке ComboBox
						}
						LSCR.CertificateFileName = CertificateFile[0];
					}
					break;
				}
				case IDC_BUTTON_SETTINGSTIMESTAMP: {
					DialogBoxParamW(PS.hInst, MAKEINTRESOURCEW(IDD_SETTINGS_TIMESTAMP_SERVERS), hDlg, SettingsTimeStampsServersDlgProc, (LPARAM)&LSCR.TimeStampServers);
					break;
				}

			}
			break;
	}
	return (INT_PTR)FALSE;
}
// Обработчик сообщений для окна "О программе".
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG: {
		HICON hDialogIcon = LoadIconW(PS.hInst, MAKEINTRESOURCEW(IDI_SIGNPROJECTTOOL));
		if (hDialogIcon == NULL)WWS::MessageError(L"Не удалось загрузить иконку для текущего диалога!", L"Ошибка загрузки иконки!", hDlg);
		else {
			SendMessageW(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hDialogIcon);
			SendMessageW(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hDialogIcon);
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

//общие функции для больинства диалогов
//данная функция проверяет существует ли строка в ListBox
INT StringExistInListBox(HWND hListBox, WSTRING str)
{
	if (hListBox != NULL) {
		if (SendMessageW(hListBox, LB_FINDSTRINGEXACT, -1, (LPARAM)str.c_str()) == LB_ERR)return FALSE;
		else return TRUE;
	}
	return -1;
}
//данная функция подсчитывает длинну полосы прокрутк
UINT CalcBItemWidth(HWND hWnd, LPCTSTR Text) {
	RECT r;
	HDC hWndDC = GetDC(hWnd);
	HDC hDC = CreateCompatibleDC(hWndDC);
	HFONT hFont = (HFONT)SendMessageW(hWnd, WM_GETFONT, 0, 0);
	HGDIOBJ hOrgFont = SelectObject(hDC, hFont);
	ZeroMemory(&r, sizeof(r));
	DrawTextW(hDC, Text, -1, &r, DT_CALCRECT | DT_SINGLELINE | DT_NOCLIP);
	SelectObject(hDC, hOrgFont);
	DeleteDC(hDC);
	ReleaseDC(hWnd, hWndDC);
	return (r.right - r.left) + (2 * GetSystemMetrics(SM_CXEDGE));
}