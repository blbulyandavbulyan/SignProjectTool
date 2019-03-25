// SignProjectTool.cpp : Определяет точку входа для приложения.
//

#include "stdafx.h"
#include "SignProjectTool.h"
#define MAX_LOADSTRING 100
//структура с основными параметрами программы
struct ProgrammSettings {
	HINSTANCE hInst;//экземпляр приложения
	WSTRINGARRAY FilesForCertification;//список файлов для подписи
	WSTRING CertificateFile;// файл сертификата для подписи им
	static const WSTRINGARRAY CommandLineValidArguments;//массив с допустимыми аргументами командной строки
	bool CertificateInCertStore = false;//хранится ли сертификат, которым будут подписываться файлы в хранилище сертификатов (задаётся при начальной настройке программы
	ALG_ID HashAlgorithmId = CALG_SHA_512;//алгоритм хеширования при подписи поумолчанию
} PP;
const WSTRINGARRAY ProgrammSettings::CommandLineValidArguments = {
		L"--about",
		L"--startup-config",
		L"--input-files",
		L"no-graphics"
};
struct LoadCetificateResult {
	bool CertificateIsLoaded = false;
	PCCERT_CONTEXT pCertContext;
	WWS_ERROR_REPORT_STRUCTW ErrorInfo;
};
struct SIGN_FILES_RESULT {
	/*ВНИМАНИЕ! ПОД ИМЕНЕМ ФАЙЛА ПОНИМАЕТСЯ ПОЛНЫЙ ПУТЬ К НЕМУ!*/
	/*****************************************************************************************************************************
	*	Общие сведения о работе функции SignFiles и о структуре SIGN_FILES_RESULT												 *
	*	Функция SignFiles подписывает несколько файлов, она вызывает функцию PArrayIteratorProc 								 *
	*	с целью получить структуру PITPROCRETURN, описывающую файл для подписи, которая содержит:								 *
	*		Имя подписываемого файла																							 *
	*		Дополнительные атрибуты для подписи(а так же информацию требуется ли их использовать								 *
	*	Если функция успешно проинициализировалась, под успешной инициализацией понимается успешная загрузка DLL Mssign32.dll	 *
	*	и успешное получение адреса функций: SignerSignEx и SignerFreeSignerContext, то функция PProgressProc будет вызываться	 *
	*	каждый раз, когда требуется обновить информацию о подписанных и неподписанных файлов, сдвинуть условный ProgressBar,	 *
	*	она будет вызываться даже в том случе, если файл неудалось подписать													 *
	*	О членах структуры SIGN_FILES_RESULT смотрите в комментариях к её членам												 *
	******************************************************************************************************************************/
	WSTRINGARRAY SigningFiles;//список имён подписанных файлов
	struct NO_SIGNING_FILE {//структура для массива с неподписанными файлами
		WSTRING NoSigningFileName;//имя неподписанного файла
		WWS_ERROR_REPORT_STRUCTW ErrorInfo;//подробная информация о причине его неподписи
	};
	struct PITPROCRETURN {//данную структуру должен возвращать итератор по именам подписываемых файлов 
		WSTRING *SigningFileName;//имя подписываемого файла
		ALG_ID HashAlgorithmId = PP.HashAlgorithmId;// алгоритм хеширования для цифровой подписи
		DWORD dwAttrChoice = 0;//требуется ли использовать pAttrAuthcode
		SIGNER_ATTR_AUTHCODE *pAttrAuthcode = nullptr;//дополнительные параметры для подписи
	};
	struct PProgressProcParameter {
		bool FileIsSigned = false;
		WWS_ERROR_REPORT_STRUCTW *ErrorInfo;
	};
	std::vector<NO_SIGNING_FILE> NoSigningFiles;//список с неподписанными файлами и подробной информацией о их неподписи
	typedef PITPROCRETURN(*PArrayIteratorProc)();//итератор по именам подписываемых файлов, возвращает структуру PITPROCRETURN
	typedef void(*PProgressProc)(PProgressProcParameter &PPP);//функция, вызываемая в случае удачного или неудачного подписывания файла, с целью обновить информацию на условном ProgressBar
	WWS_ERROR_REPORT_STRUCTW ErrorInfo;//данный член будет заполнен в том случае, если не удастся инициализировать функцию SIGN_FILES_RESULT
};
// Отправить объявления функций, включенных в этот модуль кода:
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK StartConfigurationDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AddFilesForCertificationDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
UINT CalcBItemWidth(HWND hLB, LPCTSTR Text);
#ifndef _UNICODE
UINT CalcBItemWidth(HWND hLB, PCWSTR Text);
#endif
WSTRINGARRAY OpenFiles(HWND hWnd, COMDLG_FILTERSPEC *cmf, UINT cmfSize, LPCWSTR TitleFileDialog, LPCWSTR OKButtonTitle, bool Multiselect);
SIGN_FILES_RESULT SignFiles(SIGN_FILES_RESULT::PArrayIteratorProc PItProc, SIGN_FILES_RESULT::PProgressProc PProgProc, bool DNIIASAUF);
bool ThisStringIsProgrammArgument(WSTRING arg);
void InitRegistryStorage();
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
	//Parse command line
	PP.hInst = hInstance;
	bool NoGraphicsMode = false;
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
		else if (CommandArray[i] == L"--input-files") {
			if (i != CommandArraySize - 1) {
				for (size_t j = i + 1; j < CommandArraySize; j++) {
					if (!ThisStringIsProgrammArgument(CommandArray[j].c_str())) {
						PP.FilesForCertification.push_back(CommandArray[j].c_str());
					}
					else {
						i--;
						continue;
					}
				}
			}
			else {
				MessageBox(NULL, _TEXT("Недостаточно аргументов командной строки, возможно вы забыли указать список входных файлов!"), _TEXT("Ошибка аргументов командной строки!"), MB_ICONERROR | MB_OK);
				return -1;
			}
		}
		else if (CommandArray[i] == L"no-graphics") {
			NoGraphicsMode = true;
		}
		else {
			WSTRING ErrorMessage = L"Неверный аргумент командной строки: ";
			ErrorMessage += CommandArray[i];
			MessageBoxW(NULL, ErrorMessage.c_str(), L"Неверный аргумент командной строки!", MB_ICONERROR | MB_OK);
			return -1;
		}
	}
	HMODULE hBDL = LoadLibrary(_TEXT("BDL.dll"));
	if (hBDL != NULL) {
		DLGPROC hEnterPasswordDlgProcA = (DLGPROC)GetProcAddress(hBDL, "EnterPasswordDlgProcA");
		DLGPROC hEnterPasswordDlgProcW = (DLGPROC)GetProcAddress(hBDL, "EnterPasswordDlgProcW");
		CHARVECTOR password;
		WCHARVECTOR wpassword;
		if (hEnterPasswordDlgProcA != NULL) {
			BDL::EnterPasswordDlgInitParamA EPDIA;
			EPDIA.Password = &password;
			EPDIA.Caption = "Введите пароль от сертификата:";
			EPDIA.EditPasswordCaption = "Пароль от сертификата";
			EPDIA.HasToolTip = true;
			EPDIA.ToolTipCaption[0] = "Показать пароль от сертификата";
			EPDIA.ToolTipCaption[1] = "Скрыть пароль от сертификата";
			EPDIA.OkButtonCaption = "Ввод";
			EPDIA.CancelButtonCaption = "Выход";
			EPDIA.PasswordChar = 'Ф';
			EPDIA.hIconCaption = LoadIconA(PP.hInst, MAKEINTRESOURCEA(IDI_SIGNPROJECTTOOL));
			if (EPDIA.hIconCaption == NULL)MessageError(_TEXT("Не удалось загрузить иконку для текущего диалога A!"), _TEXT("Ошибка загрузки иконки!"), NULL);
			DialogBoxParamA(hBDL, MAKEINTRESOURCEA(IDDBDL_ENTERPASSWORD), NULL, hEnterPasswordDlgProcA, (LPARAM)&EPDIA);
			MessageBoxA(NULL, password.data(), "Ваш пароль:", MB_OK | MB_ICONINFORMATION);
		}
		else MessageError(_TEXT("Не удалось получить адресс функции EnterPasswordDlgProcA из библиотеки BDL.dll!"), _TEXT("Ошибка получения адресса функции!"), NULL);
		if (hEnterPasswordDlgProcW != NULL) {
			BDL::EnterPasswordDlgInitParamW EPDIW;
			EPDIW.Password = &wpassword;
			EPDIW.Caption = L"Введите пароль от сертификата:";
			EPDIW.EditPasswordCaption = L"Пароль от сертификата";
			EPDIW.HasToolTip = true;
			EPDIW.ToolTipCaption[0] = L"Показать пароль от сертификата";
			EPDIW.ToolTipCaption[1] = L"Скрыть пароль от сертификата";
			EPDIW.OkButtonCaption = L"Ввод";
			EPDIW.CancelButtonCaption = L"Выход";
			EPDIW.PasswordChar = L'Ф';
			EPDIW.hIconCaption = LoadIconW(PP.hInst, MAKEINTRESOURCEW(IDI_SIGNPROJECTTOOL));
			if (EPDIW.hIconCaption == NULL)MessageError(_TEXT("Не удалось загрузить иконку для текущего диалога W!"), _TEXT("Ошибка загрузки иконки!"), NULL);
			DialogBoxParamW(hBDL, MAKEINTRESOURCEW(IDDBDL_ENTERPASSWORD), NULL, hEnterPasswordDlgProcW, (LPARAM)&EPDIW);
			MessageBoxW(NULL, wpassword.data(), L"Ваш пароль:", MB_OK | MB_ICONINFORMATION);
		}
		else MessageError(_TEXT("Не удалось получить адресс функции EnterPasswordDlgProcW из библиотеки BDL.dll!"), _TEXT("Ошибка получения адресса функции!"), NULL);
		FreeLibrary(hBDL);
	}
	else MessageError(_TEXT("Не удалось загрузить библиотеку BDL.dll"), _TEXT("Ошибка загрузки DLL библиотеки!"), NULL);
	// графический режим работы, в случае если графический режим отключен, данное диалоговое окно создаваться не будет
	if(!NoGraphicsMode)DialogBox(PP.hInst, MAKEINTRESOURCE(IDD_ADD_FILES_FOR_CERTIFICATION), NULL, AddFilesForCertificationDlgProc);
	//цикл перебора и подписи файлов
	
	
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
					COMDLG_FILTERSPEC cmf[5] = {//массив с фильтрами
						//фильтр для *.exe файлов
						{
							L"*.exe файлы",
							L"*.exe"
						},
						//фильтр для *.dll файлов
						{
							L"*.dll файлы",
							L"*.dll",
						},
						//фильтр для *.lib файлов
						{
							L"*.lib файлы",
							L"*.lib"
						},
						//фильтр *.cab файлов
						{
							L"*.cab файлы",
							L"*.cab"
						},
						//фильтр для *.exe, *.dll, *.lib, *.cab файлов
						{
							L"*.exe, *.dll, *.lib, *.cab файлы",
							L"*.exe;*.dll;*.lib;*.cab"
						}
					};
					WSTRINGARRAY OpenningFiles = OpenFiles(hDlg, cmf, sizeof(cmf)/sizeof(cmf[0]), L"Выберете файлы для цифровой подписи", L"Добавить...", true);
					size_t OpenningFilesSize = OpenningFiles.size();
					if (OpenningFilesSize > 0) {
						for (size_t i = 0; i < OpenningFilesSize; i++) {
							if (SendMessageW(hListSelectedFilesForCertification, LB_ADDSTRING, NULL, (WPARAM)OpenningFiles[i].c_str()) != LB_ERR) {
								UINT Temp = CalcBItemWidth(hListSelectedFilesForCertification, OpenningFiles[i].c_str());
								if (Temp > MaxStringWhidth)MaxStringWhidth = Temp;
							}
							else MessageError(_TEXT("Не удалось добавить файл!"), _TEXT("Ошибка добавления файла!"), hDlg);
						}
						SendMessage(hListSelectedFilesForCertification, LB_SETHORIZONTALEXTENT, MaxStringWhidth, 0);
						EnableWindow(hDeleteFile, TRUE);
						EnableWindow(hModifyFile, TRUE);
						EnableWindow(hSign, TRUE);
						EnableWindow(hClear, TRUE);
					}
					break;
				}
				case IDC_DELETE_FILE:{//обработки кнопки "Удалить"
					LRESULT ItemsCount = SendMessage(hListSelectedFilesForCertification, LB_GETCOUNT, NULL, NULL);// получение кол-ва элементов(файлов) в списке
					//данный код предназначен для точной проверки что всё работает правильно(ВНИМАНИЕ ЭТО ОТЛАДОЧНЫЙ КОД, В КОНЕЧНОМ ПРОЕКТЕ ОН ПРИСУТСВОВАТЬ НЕ БУДЕТ, ДЛЯ ТОГО ЧТО БЫ ЕГО НЕ БЫЛО В СОБРАННОЙ ПРОГРАММЕ СМЕНИТЕ КОНФИГУРАЦИЯ С Debug на Release)
					#ifdef _DEBUG
					OutputDebugString(_TEXT("Этап до удаления: \n"));
					TSTRINGARRAY ListBoxItems, ListBoxSelectedItems;
					for (LRESULT i = 0; i < ItemsCount; i++) {
						LRESULT ItemIsSelected = SendMessage(hListSelectedFilesForCertification, LB_GETSEL, (WPARAM)i, NULL);//получение информации о том, выбран ли элемент
						if (ItemIsSelected > 0) {// если выбран
							TCHARVECTOR ListBoxString;// здесь будет хранится полученная строка из ListBox
							LRESULT TextLen = SendMessage(hListSelectedFilesForCertification, LB_GETTEXTLEN, (WPARAM)i, NULL);//получаем длинну строки
							if (TextLen != LB_ERR) {// если не ошибка
								ListBoxString.resize(TextLen + 1);//инициализируем массив ListBoxString, к TextLen прибавляем 1, т.к. длинна, которую мы получили в предыдущем шаге не включает терминирующий ноль
								if (SendMessage(hListSelectedFilesForCertification, LB_GETTEXT, (WPARAM)i, (LPARAM)ListBoxString.data()) != LB_ERR) {//получаем текст элемента, если он был получен успешно
									ListBoxSelectedItems.push_back(ListBoxString.data());
									ListBoxItems.push_back(ListBoxString.data());
								}
								else MessageDebug(_TEXT("Не удалось получить текст выбранного элемента!"), _TEXT("Ошибка при получении текста выбранного элемента!"));//в случае неудачного получения текста элемента
							}
							else MessageDebug(_TEXT("Не удалось узнать длинну текста выбранного элемента!"), _TEXT("Ошибка получения длинны выбранного элемента!"));//в случае неудачного получения длинны строки
						}
						else if (ItemIsSelected == 0) {// если не выбран
							TCHARVECTOR ListBoxString;// здесь будет хранится полученная строка из ListBox
							LRESULT TextLen = SendMessage(hListSelectedFilesForCertification, LB_GETTEXTLEN, (WPARAM)i, NULL);//получаем длинну строки
							if (TextLen != LB_ERR) {// если не ошибка
								ListBoxString.resize(TextLen + 1);//инициализируем массив ListBoxString, к TextLen прибавляем 1, т.к. длинна, которую мы получили в предыдущем шаге не включает терминирующий ноль
								if (SendMessage(hListSelectedFilesForCertification, LB_GETTEXT, (WPARAM)i, (LPARAM)ListBoxString.data()) != LB_ERR) {//получаем текст элемента, если он был получен успешно
									ListBoxItems.push_back(ListBoxString.data());
								}
								else MessageDebug(_TEXT("Не удалось получить текст невыбранного элемента!"), _TEXT("Ошибка при получении текста невыбранного элемента!"));//в случае неудачного получения текста элемента
							}
							else MessageDebug(_TEXT("Не удалось узнать длинну текста невыбранного элемента!"), _TEXT("Ошибка получения длинны невыбранного элемента!"));//в случае неудачного получения длинны строки
						}
						else if (ItemIsSelected == LB_ERR)MessageDebug(_TEXT("Не удалось узнать выбран ли элемент"), _TEXT("Ошибка при распознавании выбранного элемента!"));//в случае ошибки проверки выбран ли элемент
					}
					OutputDebugString(_TEXT("Все элементы: \n"));
					for (LRESULT i = 0; i < ListBoxItems.size(); i++) {
						OutputDebugString(ListBoxItems[i].data());
						OutputDebugString(_TEXT("\n"));
					}
					OutputDebugString(_TEXT("Выбранные элементы: \n"));
					for (LRESULT i = 0; i < ListBoxSelectedItems.size(); i++) {
						OutputDebugString(ListBoxSelectedItems[i].data());
						OutputDebugString(_TEXT("\n"));
					}
					#endif
					///////////////////////////////////////////////////////////////////////////////////////////////
					//конец отладочного блока, дальше идёт обычной блок, выполняющий реализацию кнопки "Удалить"//
					//////////////////////////////////////////////////////////////////////////////////////////////
					if (ItemsCount != LB_ERR) {//проверка на ошибку
						MaxStringWhidth = 0;//обнуление длинны прокрутки горизонтального скролбара ListBox
						for (LRESULT i = 0; i < ItemsCount; i++) {//цикл перебора строк 
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
					//начало нового отладочного блока, данный блок будет проверять все ли выбранные элементы были удалены и не были ли удалены лишние(невыбранные) элементы
					#ifdef _DEBUG
					OutputDebugString(_TEXT("Этап после удаления: \n"));
					//начало блока для получения текущих элементов в списке
					ItemsCount = SendMessage(hListSelectedFilesForCertification, LB_GETCOUNT, NULL, NULL);// получение кол-ва элементов(файлов) в списке
					TSTRINGARRAY ListBoxElementsLastModify;
					for (LRESULT i = 0; i < ItemsCount; i++) {
						TCHARVECTOR ListBoxString;// здесь будет хранится полученная строка из ListBox
						LRESULT TextLen = SendMessage(hListSelectedFilesForCertification, LB_GETTEXTLEN, (WPARAM)i, NULL);//получаем длинну строки
						if (TextLen != LB_ERR) {// если не ошибка
							ListBoxString.resize(TextLen + 1);//инициализируем массив ListBoxString, к TextLen прибавляем 1, т.к. длинна, которую мы получили в предыдущем шаге не включает терминирующий ноль
							if (SendMessage(hListSelectedFilesForCertification, LB_GETTEXT, (WPARAM)i, (LPARAM)ListBoxString.data()) != LB_ERR) {//получаем текст элемента, если он был получен успешно
								ListBoxElementsLastModify.push_back(ListBoxString.data());
								
							}
							else MessageDebug(_TEXT("Не удалось получить текст элемента!"), _TEXT("Ошибка при получении текста элемента!"));//в случае неудачного получения текста элемента
						}
						else MessageDebug(_TEXT("Не удалось узнать длинну текста элемента!"), _TEXT("Ошибка получения длинны элемента!"));//в случае неудачного получения длинны строки
					}
					//конец блока получения текущих элементов в списке
					//начало блока проверки 
					auto ExistIdenticalElements = [](TSTRINGARRAY &a, TSTRINGARRAY &b)->TSTRINGARRAY {// данный функтор сравнивает два массива и формирует массив с элементами, существующими в обоих массивах
						TSTRINGARRAY result;
						for (LRESULT i = 0; i < a.size(); i++) {
							for (LRESULT j = 0; j < b.size(); j++) {
								if (a[i] == b[j])result.push_back(a[i].data());
							}
						}
						return result;
					};
					auto IncorrectlyDeletedItems = [](TSTRINGARRAY &A, TSTRINGARRAY &B, TSTRINGARRAY &C)->TSTRINGARRAY{
						//описание параметров:
						//A - исходный массив
						//B - массив с элементами, подлежащеми удалению
						//C - массив с дополнением A до B, требующий проверки, сформированный другим кодом 
						//данный функтор предназначен для проверки, праивльно ли пересечены A и B,
						// из массивов A и B он формирует массив, такой что в нём содержаться только те элементы из A, которых нет в B, далее он сравнивает сформированный на предыдущем шаге массив с массивом C, 
						//если эти массивы равны, то он возвращает пустой массив, если не равны, то он возвращает те элементы, которые должны быть в C(кроме тех, которые там уже есть)
						TSTRINGARRAY Result, ArrayWithCorrectlyDeletedElements;
						auto ElementIsExistInArray = [](const TSTRINGARRAY &Array, const TSTRING &element)->bool {
							for (size_t i = 0; i < Array.size(); i++) {
								if (Array[i] == element)return true;
							}
							return false;
						};
						for (LRESULT i = 0; i < A.size(); i++) {
							if (!ElementIsExistInArray(B, A[i])) {
								ArrayWithCorrectlyDeletedElements.push_back(A[i]);
							}
						}
						if (C != ArrayWithCorrectlyDeletedElements) {
							//DebugBreak();
							for (LRESULT i = 0; i < ArrayWithCorrectlyDeletedElements.size(); i++) {
								if (!ElementIsExistInArray(C, ArrayWithCorrectlyDeletedElements[i])) {
									Result.push_back(A[i]);
								}
							}
						}

						return Result;
					};
					TSTRING DebugOutputString = _TEXT("Значение списка после удаления: \n");
					for (size_t i = 0; i < ListBoxElementsLastModify.size(); i++)DebugOutputString+=(ListBoxElementsLastModify[i]+_TEXT("\n"));
					OutputDebugString(DebugOutputString.c_str());
					//ListBoxElementsLastModify.push_back(ListBoxSelectedItems[0]);
					TSTRINGARRAY NotDeletedElements = ExistIdenticalElements(ListBoxElementsLastModify, ListBoxSelectedItems);
					if (NotDeletedElements.size() > 0) {
						DebugOutputString = _TEXT("В списке содержатся неудалённые элементы: \n");
						for (size_t i = 0; i < NotDeletedElements.size(); i++)DebugOutputString += (NotDeletedElements[i] + _TEXT("\n"));
						OutputDebugString(DebugOutputString.c_str());
					}
					ListBoxElementsLastModify.pop_back();
					TSTRINGARRAY InvalidDeletedElements = IncorrectlyDeletedItems(ListBoxItems, ListBoxSelectedItems, ListBoxElementsLastModify);
					if (InvalidDeletedElements.size() > 0) {
						DebugBreak();
					}
					//конец блока проверки
					#endif
					//конец отладочного блока
					break;
				} 
				case IDC_MODIFY_FILE:{ //обработка кнопки "Изменить"
					
					break;
				}
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

UINT CalcBItemWidth(HWND hLB, LPCTSTR Text) {
	RECT r;
	HDC hLBDC = GetDC(hLB);
	HDC hDC = CreateCompatibleDC(hLBDC);
	HFONT hFont = (HFONT)SendMessage(hLB, WM_GETFONT, 0, 0);
	HGDIOBJ hOrgFont = SelectObject(hDC, hFont);
	ZeroMemory(&r, sizeof(r));
	DrawTextW(hDC, Text, -1, &r, DT_CALCRECT | DT_SINGLELINE | DT_NOCLIP);
	SelectObject(hDC, hOrgFont);
	DeleteDC(hDC);
	ReleaseDC(hLB, hLBDC);
	return (r.right - r.left) + (2 * GetSystemMetrics(SM_CXEDGE));
}
#ifndef _UNICODE
UINT CalcBItemWidth(HWND hLB, const PCWSTR Text) {
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
WSTRINGARRAY OpenFiles(HWND hWnd, COMDLG_FILTERSPEC *cmf, UINT cmfSize, LPCWSTR TitleFileDialog, LPCWSTR OKButtonTitle, bool Multiselect) {
	WSTRINGARRAY FilesNames;
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
		COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr)) {
		IFileOpenDialog *pFileOpen;
		// Create the FileOpenDialog object.
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
			IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));
		if (SUCCEEDED(hr)) {
			// Show the Open dialog box.
			hr = pFileOpen->SetFileTypes(cmfSize, cmf);
			if (SUCCEEDED(hr)) {
				hr = pFileOpen->SetTitle(TitleFileDialog);
				if (hr != S_OK)MessageError(_TEXT("Не удалось установить заголовок для диалогового окна открытия файлов!"), _TEXT("Ошибка установки заголовка!"), hWnd, hr);
				hr = pFileOpen->SetOkButtonLabel(OKButtonTitle);
				if (hr != S_OK)MessageError(_TEXT("Не удалось установить текст кнопки OK для диалогового окна открытия файлов!"), _TEXT("Ошибка установки текста кнопки OK!"), hWnd, hr);
				hr = pFileOpen->SetOptions(FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST | (Multiselect == TRUE ? FOS_ALLOWMULTISELECT : 0));
				if (SUCCEEDED(hr)) {
					hr = pFileOpen->Show(hWnd);
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
											FilesNames.push_back(pszFilePath);
											CoTaskMemFree(pszFilePath);
										}
										else MessageError(_TEXT("Не удалось получить имя открытого вами файла, файл не будет добавлен!"), _TEXT("Ошибка получения имени открытого файла!"), hWnd, hr);
										MyFile->Release();
									}
									else MessageError(_TEXT("Не удалось получить один из открытых вами файлов, файл не будет добавлен!"), _TEXT("Ошибка получения открытого файла!"), hWnd, hr);
								}
							}
							else MessageError(_TEXT("Не удалось получить количество открытых вами файлов!"), _TEXT("Ошибка получения кол-ва открытых файлов!"), hWnd, hr);
							pItem->Release();
						}
						else MessageError(_TEXT("Не удалось получить открытые вами файлы!"), _TEXT("Ошибка получения открытых вами файлов!"), hWnd, hr);
					}
					else if (hr != HRESULT_FROM_WIN32(ERROR_CANCELLED)) MessageError(_TEXT("Не удалось показать диалоговое окно открытия файлов, дальнейшее открытие файлов невозможно!"), _TEXT("Ошибка показа диалогового окна!"), hWnd, hr);
					pFileOpen->Release();
				}
				else MessageError(_TEXT("Не удалось установить необходимые опции для файлового диалога, дальнейшее открытие файлов невозможно!"), _TEXT("Ощибка установки опций!"), hWnd, hr);
			}
			else MessageError(_TEXT("Не удалось установить фильтры допустимых файлов, невозможно добавить файлы!"), _TEXT("Ошибка установки фильтров для файлового диалога!"), hWnd, hr);
		}
		else MessageError(_TEXT("Не удалось создать объект CLSID_FileOpenDialog, дальнейшее открытие файлов невозможно!"), _TEXT("Ошибка создания объекта CLSID_FileOpenDialog!"), hWnd, hr);
		CoUninitialize();
	}
	else MessageError(_TEXT("Не удалось инициализировать библиотеку COM!"), _TEXT("Ошибка инициализиции библиотеки COM!"), hWnd, hr);
	return FilesNames;
}
LoadCetificateResult LoadCertificate(const WSTRING *CertificateHref, bool LoadCertificateFromCertStore) {
	LoadCetificateResult LCR;
	PCCERT_CONTEXT pCertContext = nullptr;
	HCERTSTORE hCertStore = nullptr;
	if (PP.CertificateInCertStore) {
		//процедура загрузки сертификата для подписи из хранилища
		WSTRING CertSubject = L"", CertStoreName = L"";
		for (size_t i = PP.CertificateFile.size() - 1; i >= 0; i--) {
			if (CertificateHref->operator[](i) != L'/') {
				CertSubject.push_back(CertificateHref->operator[](i));
			}
			else {
				for (size_t j = (i != 0 ? i - 1 : i); j >= 0; j--) {
					CertStoreName.push_back(CertificateHref->operator[](i));
					if (j == 0)goto AndParseCertInCertStoreNameAndCertSubject;
				}
			}
			if (i == 0)goto AndParseCertInCertStoreNameAndCertSubject;
		}
		AndParseCertInCertStoreNameAndCertSubject:
		hCertStore = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, NULL, CERT_SYSTEM_STORE_CURRENT_USER, (void *)CertStoreName.c_str());
		if (hCertStore) {
			pCertContext = CertFindCertificateInStore(hCertStore, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 0, CERT_FIND_SUBJECT_STR, (void *)CertSubject.c_str(), NULL);
			if (!pCertContext) {
				WSTRING ErrorText = L"Не удалось открыть сертификат \"" + CertSubject + L"\" из хранилища сертификатов сертификатов \"" + CertStoreName + L"\"!";
				LCR.ErrorInfo = ErrorString(ErrorText.c_str());
				LCR.ErrorInfo.ErrorCaption = L"Ошибка открытия сертификата!";
				goto cleanup;
			}
		}
		else {
			WSTRING ErrorText = L"Не удалось открыть хранилище сертификатов \"" + CertStoreName + L"\" в котором должен был лежать сертификат для подписи!";
			LCR.ErrorInfo = ErrorString(ErrorText.c_str());
			LCR.ErrorInfo.ErrorCaption = L"Ошибка открытия хранилища сертификатов!";
			goto cleanup;
		}
	}
	else {
		//процедура загрузки сертификата для подписи из файла
		CRYPTUI_WIZ_IMPORT_SRC_INFO GettingCertFromFile;
		memset(&GettingCertFromFile, 0, sizeof(CRYPTUI_WIZ_IMPORT_SRC_INFO));
		GettingCertFromFile.dwSize = sizeof(CRYPTUI_WIZ_IMPORT_SRC_INFO);
		GettingCertFromFile.dwSubjectChoice = CRYPTUI_WIZ_IMPORT_SUBJECT_FILE;
		GettingCertFromFile.pwszFileName = PP.CertificateFile.c_str();
		GettingCertFromFile.pwszPassword = L"";
		
		GettingCertFromFile.dwFlags = CRYPT_EXPORTABLE | CRYPT_USER_PROTECTED;
		if (CryptUIWizImport(CRYPTUI_WIZ_NO_UI, NULL, NULL, &GettingCertFromFile, NULL) != 0) {
			LCR.pCertContext = GettingCertFromFile.pCertContext;
		}
		else {
			WSTRING ErrorText = L"Не удалось загрузить сертификат для подписи из файла \"" + PP.CertificateFile + L"\"";
			LCR.ErrorInfo = ErrorString(ErrorText.c_str());
			LCR.ErrorInfo.ErrorCaption = L"Ошибка загрузки сертификата для подписи из файла!";
			goto cleanup;
		}
	}
	cleanup:
	if (pCertContext)CertFreeCertificateContext(pCertContext);
	if (hCertStore)CertCloseStore(hCertStore, CERT_CLOSE_STORE_CHECK_FLAG);
	return LCR;
}
/*
Данная функция подписывает файлы цифровым сертификатом, в качестве параметров она принимает:
hWnd - дескриптор окна, которому будут принадлежать диалоговые окна, выводящиеся по мере необходимости
PItProc - указатель на процедуру итератор(процедура итератор возвращает имя файла, она вызывается каждый раз, когда требуется следующее имя файла,
	если больше не нужно подписывать файлы, функция должна вернуть пустую строку
PProgProc - данная функция вызывается в том случае, если требуется изменить тот объект, который отображает прогресс выполненной операции,
	она вызывается даже в том случае, если конретный файл не удалось подписать.
DNIIASAUF - данная переменная говорит о том не нужно ли включать в возвращаемое значение фукнции SignFiles информацию о подписанных и неподписанных файлах, установите её в true
если информацию о подписанных и неподписанных файлах не нужно включать в результат, в противном случае false
Возвращаемое значение: 
	возращает структуру SIGN_FILES_RESULT, в ней содержится список подписанных, неподписанных файлов (вместе с именами неподписанных файлов храняться так же причины их неподписи),
	так же в этой структуре содержится информация о том успешно ли завершилась инициализация функции, подробности об этой структуре смотрите в комментариях к её определению
*/

SIGN_FILES_RESULT SignFiles(SIGN_FILES_RESULT::PArrayIteratorProc PItProc, SIGN_FILES_RESULT::PProgressProc PProgProc, bool DNIIASAUF) {
	SIGN_FILES_RESULT result;
	HMODULE hMssign32 = LoadLibrary(_TEXT("Mssign32.dll"));// загрузка необходимой для работы данной функции DLL
	PCCERT_CONTEXT pCertContext = nullptr;
	HCERTSTORE hCertStore = nullptr;
	SIGNER_FILE_INFO signerFileInfo;
	SIGNER_SUBJECT_INFO signerSubjectInfo;
	SIGNER_CERT_STORE_INFO signerCertStoreInfo;
	SIGNER_CERT signerCert;
	SIGNER_SIGNATURE_INFO signerSignatureInfo;
	SIGNER_CONTEXT * pSignerContext = NULL;
	if (hMssign32) {
		SignerFreeSignerContextType pfSignerFreeSignerContext = (SignerFreeSignerContextType)GetProcAddress(hMssign32, "SignerFreeSignerContext");
		if (pfSignerFreeSignerContext) {
			SignerSignExType pfSignerSignEx = (SignerSignExType)GetProcAddress(hMssign32, "SignerSignEx");
			if (pfSignerSignEx) {
				//начало реализации алгоритма цифровой подписи файлов
				if (PP.CertificateInCertStore) {
					//процедура загрузки сертификата для подписи из хранилища
					WSTRING CertSubject = L"", CertStoreName = L"";
					for (size_t i = PP.CertificateFile.size() - 1; i >= 0; i--) {
						if (PP.CertificateFile[i] != L'/') {
							CertSubject.push_back(PP.CertificateFile[i]);
						}
						else {
							for (size_t j = (i != 0 ? i - 1 : i); j >= 0; j--) {
								CertStoreName.push_back(PP.CertificateFile[i]);
								if (j == 0)goto AndParseCertInCertStoreNameAndCertSubject;
							}
						}
						if (i == 0)goto AndParseCertInCertStoreNameAndCertSubject;
					}
					AndParseCertInCertStoreNameAndCertSubject:
					hCertStore = CertOpenStore(CERT_STORE_PROV_SYSTEM, 0, NULL, CERT_SYSTEM_STORE_CURRENT_USER, (void *)CertStoreName.c_str());
					if (hCertStore) {
						pCertContext = CertFindCertificateInStore(hCertStore, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 0, CERT_FIND_SUBJECT_STR, (void *)CertSubject.c_str(), NULL);
						if (!pCertContext) {
							WSTRING ErrorText = L"Не удалось открыть сертификат \"" + CertSubject + L"\" из хранилища сертификатов сертификатов \"" + CertStoreName + L"\"!";
							result.ErrorInfo = ErrorString(ErrorText.c_str());
							result.ErrorInfo.ErrorCaption = L"Ошибка открытия сертификата!";
							goto cleanup;
						}
					}
					else {
						WSTRING ErrorText = L"Не удалось открыть хранилище сертификатов \"" + CertStoreName + L"\" в котором должен был лежать сертификат для подписи!";
						result.ErrorInfo = ErrorString(ErrorText.c_str());
						result.ErrorInfo.ErrorCaption = L"Ошибка открытия хранилища сертификатов!";
						goto cleanup;
					}
				}
				else {
					//процедура загрузки сертификата для подписи из файла
					CRYPTUI_WIZ_IMPORT_SRC_INFO GettingCertFromFile;
					memset(&GettingCertFromFile, 0, sizeof(CRYPTUI_WIZ_IMPORT_SRC_INFO));
					GettingCertFromFile.dwSize = sizeof(CRYPTUI_WIZ_IMPORT_SRC_INFO);
					GettingCertFromFile.dwSubjectChoice = CRYPTUI_WIZ_IMPORT_SUBJECT_FILE;
					GettingCertFromFile.pwszFileName = PP.CertificateFile.c_str();
					GettingCertFromFile.pwszPassword = L"";
					GettingCertFromFile.dwFlags = CRYPT_EXPORTABLE | CRYPT_USER_PROTECTED;
					if (CryptUIWizImport(CRYPTUI_WIZ_NO_UI, NULL, NULL, &GettingCertFromFile, NULL) != 0) {
						pCertContext = GettingCertFromFile.pCertContext;
					}
					else {
						WSTRING ErrorText = L"Не удалось загрузить сертификат для подписи из файла \"" + PP.CertificateFile + L"\"";
						result.ErrorInfo = ErrorString(ErrorText.c_str());
						result.ErrorInfo.ErrorCaption = L"Ошибка загрузки сертификата для подписи из файла!";
						goto cleanup;
					}
				}
				if (pCertContext != nullptr) {
					signerFileInfo.cbSize = sizeof(SIGNER_FILE_INFO);
					// Prepare SIGNER_SUBJECT_INFO struct
					signerSubjectInfo.cbSize = sizeof(SIGNER_SUBJECT_INFO);
					signerSubjectInfo.pdwIndex = NULL;
					signerSubjectInfo.dwSubjectChoice = 1; // SIGNER_SUBJECT_FILE
					signerSubjectInfo.pSignerFileInfo = &signerFileInfo;
					// Prepare SIGNER_CERT_STORE_INFO struct
					signerCertStoreInfo.cbSize = sizeof(SIGNER_CERT_STORE_INFO);
					signerCertStoreInfo.pSigningCert = pCertContext;
					signerCertStoreInfo.dwCertPolicy = 2; // SIGNER_CERT_POLICY_CHAIN
					signerCertStoreInfo.hCertStore = NULL;
					// Prepare SIGNER_CERT struct
					signerCert.cbSize = sizeof(SIGNER_CERT);
					signerCert.dwCertChoice = 2; // SIGNER_CERT_STORE
					signerCert.pCertStoreInfo = &signerCertStoreInfo;
					signerCert.hwnd = NULL;
					// Prepare SIGNER_SIGNATURE_INFO struct
					signerSignatureInfo.cbSize = sizeof(SIGNER_SIGNATURE_INFO);
					signerSignatureInfo.algidHash = PP.HashAlgorithmId;
					signerSignatureInfo.dwAttrChoice = 0; // SIGNER_NO_ATTR
					signerSignatureInfo.pAttrAuthcode = NULL;
					signerSignatureInfo.psAuthenticated = NULL;
					signerSignatureInfo.psUnauthenticated = NULL;
					signingalgbegin:
					SIGN_FILES_RESULT::PITPROCRETURN PITResult = PItProc();//получение имени файла с помощью функции итератора, определяемой пользователем
					if (*PITResult.SigningFileName != L"") {//если имя не пустое
						HANDLE hFileForSigning = CreateFileW(PITResult.SigningFileName->c_str(), GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);//открываем файл для цифровой подписи
						if (hFileForSigning) {
							//процедура подписи файла сертификатом
							signerFileInfo.pwszFileName = PITResult.SigningFileName->c_str();
							signerFileInfo.hFile = hFileForSigning;
							signerSignatureInfo.dwAttrChoice = PITResult.dwAttrChoice; // SIGNER_NO_ATTR
							signerSignatureInfo.pAttrAuthcode = PITResult.pAttrAuthcode;
							HRESULT hSignerSignResult = pfSignerSignEx(0, &signerSubjectInfo, &signerCert, &signerSignatureInfo, NULL, NULL, NULL, NULL, &pSignerContext);
							if (SUCCEEDED(hSignerSignResult)) {
								SIGN_FILES_RESULT::PProgressProcParameter ppp;
								ppp.FileIsSigned = true;
								PProgProc(ppp);
								if (!DNIIASAUF)result.SigningFiles.push_back(*PITResult.SigningFileName);
								pfSignerFreeSignerContext(pSignerContext);
							}
							else {
								//в случае если неудалось подписать файл
								WSTRING ErrorText = L"Не удалось подписать файл \"" + *PITResult.SigningFileName + L"\" цифровой подписью!";
								WWS_ERROR_REPORT_STRUCTW ErrorInfo = ErrorString(ErrorText.c_str());
								SIGN_FILES_RESULT::PProgressProcParameter ppp;
								ErrorInfo.ErrorCaption = L"Не удалось подписать файл!";
								ppp.ErrorInfo = &ErrorInfo;
								PProgProc(ppp);
								if (!DNIIASAUF) {
									SIGN_FILES_RESULT::NO_SIGNING_FILE NSF;
									NSF.ErrorInfo = ErrorInfo;
									NSF.NoSigningFileName = *PITResult.SigningFileName;
									result.NoSigningFiles.push_back(NSF);
								}
							}
						}
						else {
							//в случае если не удалось открыть файл для подписи
							SIGN_FILES_RESULT::NO_SIGNING_FILE NSF;
							WSTRING ErrorText = L"Не удалось открыть файл \"" + *PITResult.SigningFileName + L"\" для цифровой подписи!";
							WWS_ERROR_REPORT_STRUCTW ErrorInfo = ErrorString(ErrorText.c_str());
							ErrorInfo.ErrorCaption = L"Не удалось открыть файл для подписи!";
							SIGN_FILES_RESULT::PProgressProcParameter ppp;
							ppp.ErrorInfo = &ErrorInfo;
							PProgProc(ppp);
							if (!DNIIASAUF) {
								SIGN_FILES_RESULT::NO_SIGNING_FILE NSF;
								NSF.ErrorInfo = ErrorInfo;
								NSF.NoSigningFileName = *PITResult.SigningFileName;
								result.NoSigningFiles.push_back(NSF);
							}
						}
						goto signingalgbegin;
					}
				}
				else {
					result.ErrorInfo.ErrorString = L"Произошла неизвестная ошибка при попытке получения сертификата, указатель на сертификат всё ещё nullptr!";
					result.ErrorInfo.ErrorCaption = L"Ошибка получения сертификата!";
					goto cleanup;
				}
			}
			else {
				result.ErrorInfo = ErrorString(L"Не удалось получить адрес функции SignerSignEx, невозможно продолжить работу!");
				result.ErrorInfo.ErrorCaption = L"Ошибка получения адреса функции!";
				FreeLibrary(hMssign32);
			}
		}
		else {
			result.ErrorInfo = ErrorString(L"Не удалось получить адрес функции SignerFreeSignerContext, невозможно продолжить работу");
			result.ErrorInfo.ErrorCaption = L"Ошибка получения адреса функции!";
			FreeLibrary(hMssign32);
		}
	}
	else {
		result.ErrorInfo = ErrorString(L"Не удалось загрузить библиотеку Mssign32.dll, невозможно продолжить работу!");
		result.ErrorInfo.ErrorCaption = L"Ошибка загрузки бибилиотеки!";
	}
	cleanup:
	if (hMssign32)FreeLibrary(hMssign32);
	if (pCertContext)CertFreeCertificateContext(pCertContext);
	if (hCertStore)CertCloseStore(hCertStore, CERT_CLOSE_STORE_CHECK_FLAG);
	return result;
}
bool ThisStringIsProgrammArgument(WSTRING arg) {
	for (size_t i = 0; i < ProgrammSettings::CommandLineValidArguments.size(); i++)if (arg == ProgrammSettings::CommandLineValidArguments[i])return true;
	return false;
}
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