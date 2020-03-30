// SignProjectTool.cpp : Определяет точку входа для приложения.
//
#include "stdafx.h"
#define MAX_LOADSTRING 100
using namespace WWS;
//список допустимых аргументов командной строки
const WSTRINGARRAY ProgrammSettings::CommandLineValidArguments = {
		L"--about",L"/a",//вызов диалога о программе
		L"--startup-config",L"/sc",//вызов диалога начальной конфигурации программы
		L"--input-files",L"/i",// указать входные для подписи файлы
		L"--no-graphics",L"/ng",//отключить графический интерфейс
		L"--not-load-settings-from-registry",L"/nlsfr",L"/nls",//не загружать настройки из реестра Windows
		L"--not-save-settings-in-registry",L"/nssir",L"/nss",//не сохранять настройки в реестр Windows
		L"--not-use-registry",L"/nur",//не использовать реестр Windows, данный аргумент делает тоже что и аргументы, в предыдущих двух строках
		L"--certificate-file",L"/c",L"/cf",//путь к файлу сертификата, для подписи
		L"--alghash",L"/ah"//алгоритм хеширования, для подписи
};
ProgrammSettings PP;//глобальный экземпляр структуры с параметрами программы
ProgrammStatement PS;// глобальный экземпляр структуры с состоянием программы
//данные функции используются в структурах, которые описаны ниже, поэтому их прототипы на самом верху
bool CheckBit(unsigned word, unsigned bit);
INT_PTR CALLBACK AddFilesForCertificationDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);// процедура обработки сообщений диалога IDD_ADD_FILES_FOR_CERTIFICATION
bool ThisStringIsProgrammArgument(WSTRING arg);//проверка является ли строка аргументом программы
//BOOL CALLBACK FindListBoxChildWindowProc(HWND hWnd, LPARAM lParam);//процедура обратного вызова для функции EnumChildWindows, её цель - найти ListBox в дочерних окнах и вернуть на него первый попавшейся дескриптор, в качестве lParam ожидает указаьтель на переменную типа HWND для записи в неё результата, используется с EnumChildWindows
BOOL WINAPI ConsoleHandler(_In_ DWORD dwCtrlType);
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow){
    UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);
	unsigned sfLoadParametersFromRegistry = ProgrammSettings::REGISTRY_QUERY::ALG_HASH | ProgrammSettings::REGISTRY_QUERY::SIGN_CERTIFICATE_FILE_NAME;//данная переменная хранит в себе то,
	//какие параметры из реестра следует загрузить, по умолчанию загружаются все парметры, однако, если указан аргумент командной строки, 
	//который задаёт соотвествующий параметр, то предпочитается он, те параметры которые взяты из командной строки исключаеются отсюда посредство исключающего или 
	/*Обработка аргументов командной строки*/
	PS.hInst = hInstance;
	bool NoGraphicsMode = false;
	WSTRINGARRAY CommandArray = BreakAStringIntoAnArrayOfStringsByCharacter(lpCmdLine, L' ');
	size_t CommandArraySize = CommandArray.size();
	for (size_t i = 0; i < CommandArraySize; i++) {
		if (CommandArray[i] == L"--about" || CommandArray[i] == L"/a") {
			DialogBoxW(PS.hInst, MAKEINTRESOURCEW(IDD_ABOUTBOX), NULL, About);
			return NULL;
		}
		else if (CommandArray[i] == L"--startup-config" || CommandArray[i] == L"/sc") {
			//показ диалога с настройками и премениение измениений

		}
		else if (CommandArray[i] == L"--not-load-settings-from-registry" || CommandArray[i] == L"/nlsfr" || CommandArray[i] == L"/nls") {
			PP.LoadSettingsFromRegistry = false;
		}
		else if (CommandArray[i] == L"--not-save-settings-in-registry" || CommandArray[i] == L"/nssir" || CommandArray[i] == L"/nss") {
			PP.SaveSettingsInRegistry = false;
		}
		else if (CommandArray[i] == L"--not-use-registry" || CommandArray[i] ==  L"/nur") {
			PP.SaveSettingsInRegistry = false;
			PP.LoadSettingsFromRegistry = false;
		}
		else if (CommandArray[i] == L"--input-files" || CommandArray[i] == L"/i") {
			//входные файла для подписи
			if (i != (CommandArraySize - 1)) {
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
				MessageBoxW(NULL, L"Недостаточно аргументов командной строки, возможно вы забыли указать список входных файлов!", L"Ошибка аргументов командной строки!", MB_ICONERROR | MB_OK);
				return -1;
			}
		}
		else if (CommandArray[i] == L"--certificate-file" || CommandArray[i] == L"/c" || CommandArray[i] == L"/cf") {
			//получение пути к файлу сертификата для подписи
			if (i != (CommandArraySize - 1)) {
				WSTRING CertificateFile = CommandArray[i + 1];
				if (!ThisStringIsProgrammArgument(CertificateFile)) {
					if (FileReadebleExists(CertificateFile.c_str())) {
						PP.CertificateFile = CertificateFile;
						sfLoadParametersFromRegistry ^= ProgrammSettings::REGISTRY_QUERY::SIGN_CERTIFICATE_FILE_NAME;
					}
					else {
						WSTRING ErrorText = L"Файла по пути \"" + CertificateFile + L"\" не существует или он не доступен для чтения!";
						MessageBoxW(NULL, ErrorText.c_str(), L"Ошибка октрытия файла!", MB_OK | MB_ICONERROR);
						return -1;
					}
				}
				else {
					MessageBoxW(NULL, L"Имя файла не может являтся аргументом командной строки данной программы!", L"Неверное имя файла!", MB_OK | MB_ICONERROR); 
					return -1;
				}

			}
			else {
				MessageBoxW(NULL, L"Недостаточно аргументов командной строки, возможно вы забыли указать полный путь к сертификату!", L"Ошибка обработки аргументов командной строки!", MB_ICONERROR | MB_OK);
				return -1;
			}
		}
		else if (CommandArray[i] == L"--alghash" || CommandArray[i] == L"/ah") {
			//получения алгоритма хеширования для цифровой подписи
		}
		else if (CommandArray[i] == L"--no-graphics" || CommandArray[i] == L"/ng") {
			NoGraphicsMode = true;
		}
		else {
			WSTRING ErrorMessage = L"Неверный аргумент командной строки: ";
			ErrorMessage += CommandArray[i];
			MessageBoxW(NULL, ErrorMessage.c_str(), L"Неверный аргумент командной строки!", MB_ICONERROR | MB_OK);
			return -1;
		}
	}
	if(PP.LoadSettingsFromRegistry)PP.LoadProgrammSettingsOfRegistry(sfLoadParametersFromRegistry);//загрузка требуемых параметров из реестра перед дальнейшей работой
	HACCEL hAccel = LoadAcceleratorsW(PS.hInst, MAKEINTRESOURCEW(IDC_SIGNPROJECTTOOL));
	MSG msg = { 0 };
	if (hAccel) {
		PS.hRootWnd = CreateDialogW(PS.hInst, MAKEINTRESOURCEW(IDD_ADD_FILES_FOR_CERTIFICATION), NULL, AddFilesForCertificationDlgProc);
		if (PS.hRootWnd) {
			ShowWindow(PS.hRootWnd, nCmdShow);
			UpdateWindow(PS.hRootWnd);
			while (GetMessageW(&msg, NULL, 0, 0)) {
				if (!TranslateAcceleratorW(PS.hRootWnd, hAccel, &msg)) {
					if (!IsDialogMessageW(PS.hRootWnd, &msg)) {
						TranslateMessage(&msg);
						DispatchMessageW(&msg);
					}
				}
			}
		}
		else {
			MessageError(L"Не удалось создать главное окно программы!", L"Ошибка создания главного окна программы!", NULL);
			return FALSE;
		}
	}
	else {
		MessageError(L"Не удалось загрузить таблицу асселераторов!", L"Ошибка загрузки таблицы асселераторов!", NULL);
		return FALSE;
	}
	PS.hRootWnd = NULL;
	//if (PP.CertificateFile != L"")PP.SaveProgrammSettingsInRegistry(ProgrammSettings::REGISTRY_QUERY::SIGN_CERTIFICATE_FILE_NAME | ProgrammSettings::REGISTRY_QUERY::ALG_HASH);
	if(PP.SaveSettingsInRegistry)PP.SaveProgrammSettingsInRegistry(ProgrammSettings::REGISTRY_QUERY::ALG_HASH);
	return msg.wParam;
}
// Функция обработки сообщений диалога IDD_ADD_FILES_FOR_CERTIFICATION
INT_PTR CALLBACK AddFilesForCertificationDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	HWND hListSelectedFilesForCertification = GetDlgItem(hDlg, IDC_LIST_SELECTED_FILES_FOR_CERTIFICTION), hDeleteFile = GetDlgItem(hDlg, IDC_DELETE_FILE), hModifyFile = GetDlgItem(hDlg, IDC_MODIFY_FILE), hSign = GetDlgItem(hDlg, IDC_SIGN), hClear = GetDlgItem(hDlg, IDC_CLEAR), hQuitCancel = GetDlgItem(hDlg, IDCANCEL), hAddFiles = GetDlgItem(hDlg, IDC_ADD_FILE), hPauseSigning = GetDlgItem(hDlg, IDC_PAUSE_SIGNING);
	static HWND hStatusBar = NULL, hToolBar = NULL;
	static UINT MaxStringWhidth = 0;//переменная характеризует максимально возможное значение, на которое можно прокрутить горизонтальный скролбар ListBox
	//static BOOL ConsoleIsAlloced = FALSE, SigningOperation = FALSE;
	static DWORD ThreadSigningFilesId = NULL;
	static PARAMETERS_FOR_THREAD_SIGN_FILES TSFINIT;
	static BOOL ThreadForSigningFilesIsSuspend = FALSE;
	static HTHEME DefaultThemeForProgressBar = NULL;
	static bool RemoveSuccessfullySignedFilesFromTheList = false;
	switch (message) {
		case WM_INITDIALOG:{
			HICON hDialogIcon = LoadIconW(PS.hInst, MAKEINTRESOURCEW(IDI_SIGNPROJECTTOOL));//загрузка иконки диалога из ресурсов
			if (hDialogIcon == NULL)MessageError(L"Не удалось загрузить иконку для текущего диалога!", L"Ошибка загрузки иконки!", hDlg);
			else {
				//установка иконки диалога
				SendMessageW(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hDialogIcon);
				SendMessageW(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hDialogIcon);
			}
			hStatusBar = CreateStatusWindowW(WS_VISIBLE | WS_CHILD, L"Ничего не запланировано", hDlg, 1);
			RECT DialogRect = {0};
			GetWindowRect(hDlg, &DialogRect);
			int StatusBarBordersArray[2];
			StatusBarBordersArray[0] = abs(DialogRect.left - DialogRect.right)/2;
			StatusBarBordersArray[1] = abs(DialogRect.left - DialogRect.right);
			SendMessage(hStatusBar, SB_SETPARTS, 2, (LPARAM)StatusBarBordersArray);
			SendMessage(hStatusBar, SB_SETTEXTW, MAKEWORD(1, SBT_POPOUT), (LPARAM)L"Сертификат не выбран");
			/*HWND hProgressForSigningFiles = GetDlgItem(hDlg, IDC_PROGRESS_SIGNING_FILES);
			SetWindowTheme(hProgressForSigningFiles, L"Explorer", L"");
			SendMessageW(hProgressForSigningFiles, PBM_SETBARCOLOR, 0, RGB(255, 0, 0));*/
			//hToolBar = CreateToolbarEx(hDlg, NULL, ID_TOOLBAR_CONTROL_FOR_ADD_FILES_FOR_CERTIFICATION, 2, PS.hInst, )
			return (INT_PTR)TRUE;
		}
		case WM_DESTROY: {
			if (PS.hSignFilesThread != NULL) {
				DWORD result = WaitForSingleObject(PS.hSignFilesThread, 0);
				if (result != WAIT_OBJECT_0) {
					// the thread handle is signaled - the thread has terminated
					PostThreadMessageW(ThreadSigningFilesId, PARAMETERS_FOR_THREAD_SIGN_FILES::THREAD_MESSAGES::THM_CLOSE, 0, 0);
					if (ThreadForSigningFilesIsSuspend)ResumeThread(PS.hSignFilesThread);
				}
				if (CloseHandle(PS.hSignFilesThread) == NULL)MessageError(L"Не удаллось закрыть дескриптор потока подписи файлов!", L"Ошибка закрытия дескриптора!", hDlg);
			}
			PostQuitMessage(LOWORD(wParam));
			return 0;
		}
		//обработка показа контекстного меню
		case WM_CONTEXTMENU: {
			//контекстное меню над списком с файлами, требующими цифровую подпись
			if ((HWND)wParam == hListSelectedFilesForCertification) {
				if (SendMessageW(hListSelectedFilesForCertification, LB_GETCOUNT, NULL, NULL) > 0) {
					long x = LOWORD(lParam);
					long y = HIWORD(lParam);
					POINT p = { x, y };
					//ClientToScreen(hDlg, &p); //add this line
					HMENU hMenu = CreatePopupMenu();
					int pos = LBItemFromPt(hListSelectedFilesForCertification, p, 0);
					if (pos != -1)SendMessageW(hListSelectedFilesForCertification, LB_SETSEL, TRUE, pos);
					LRESULT SelectedElementsCount = SendMessageW(hListSelectedFilesForCertification, LB_GETSELCOUNT, NULL, NULL);
					LRESULT ElementsCount = SendMessageW(hListSelectedFilesForCertification, LB_GETCOUNT, NULL, NULL);
					if (SelectedElementsCount >= 2) {
						AppendMenuW(hMenu, MFT_STRING, IDC_DELETE_FILE, L"Удалить выбранные"); 
						AppendMenuW(hMenu, MFT_SEPARATOR, 0, NULL);
						AppendMenuW(hMenu, MFT_STRING, IDM_CLEAR_SELECT_IN_LIST, L"Очистить выбор");
					}
					else AppendMenuW(hMenu, MFT_STRING, IDC_DELETE_FILE, L"&Удалить");
					AppendMenuW(hMenu, MFT_SEPARATOR, 0, NULL);
					if (ElementsCount != SelectedElementsCount) { 
						AppendMenuW(hMenu, MFT_STRING, IDM_SELECT_ALL_IN_LIST, L"Выделить всё");
						AppendMenuW(hMenu, MFT_SEPARATOR, 0, NULL);
					}
					AppendMenuW(hMenu, MFT_STRING, IDM_SHOW_FILE_IN_EXPLORER, L"Показать в проводнике");
					TrackPopupMenu(hMenu, TPM_RIGHTBUTTON |
						TPM_TOPALIGN |
						TPM_LEFTALIGN,
						LOWORD(lParam),
						HIWORD(lParam), 0, hDlg, NULL);
					DestroyMenu(hMenu);
				}
			}
			break;
		}
		case WM_COMMAND:
			switch (HIWORD(wParam)) {
				case LBN_SELCHANGE: {
					switch (LOWORD(wParam)) {
						case IDC_LIST_SELECTED_FILES_FOR_CERTIFICTION: {
							LRESULT ItemsCountSelected = SendMessageW(hListSelectedFilesForCertification, LB_GETSELCOUNT, NULL, NULL);// получение кол-ва выбранных элементов(файлов) в списке
							if (ItemsCountSelected != LB_ERR) {
								if (ItemsCountSelected > 0)EnableWindow(hDeleteFile, TRUE);
								else EnableWindow(hDeleteFile, FALSE);
							}
							else MessageError(L"Не удалось получить количество выбранных элементов", L"Ошибка получения количества элементов!", hDlg);//в случае ошибки получения кол-ва элементов
							break;
						}
					}
					break;
				}
				break;
			}
			switch (LOWORD(wParam)) {
				case IDM_STARTUPCONFIG: {
					START_CONFIGURATION_RESULT SCR;
					if (DialogBoxParamW(PS.hInst, MAKEINTRESOURCEW(IDD_START_CONFIGURATION), NULL, StartConfigurationDlgProc, (LPARAM)&SCR) == IDOK) {
						LRESULT ItemsCount = SendMessageW(hListSelectedFilesForCertification, LB_GETCOUNT, NULL, NULL);// получение кол-ва элементов(файлов) в списке
						if (SCR.CertificateFileName != L"") {
							SendMessageW(hStatusBar, SB_SETTEXTW, MAKEWORD(1, SBT_POPOUT), (LPARAM)L"Сертификат выбран"); 
							if (ItemsCount > 0)EnableWindow(hSign, TRUE);
							PP.CertificateFile = SCR.CertificateFileName;
						}
						if (SCR.HashAlgoritmId != NULL)PP.HashAlgorithmId = SCR.HashAlgoritmId;
					}
					break;
				}
				case IDM_ABOUT: //обработка пункта меню "О программе"
					DialogBoxW(PS.hInst, MAKEINTRESOURCEW(IDD_ABOUTBOX), hDlg, About);
					break;
				case IDC_SIGN:{//обработка кнопки "Подписать"
					TSFINIT.ItemsCount = SendMessageW(hListSelectedFilesForCertification, LB_GETCOUNT, NULL, NULL);// получение кол-ва элементов(файлов) в списке
					if (TSFINIT.ItemsCount != LB_ERR) {
						HMODULE hBDL = LoadLibraryW(L"BDL.dll"), hMssign32 = LoadLibrary(L"Mssign32.dll");
						if (hBDL) {
							if (hMssign32 != NULL) {
								SignerSignType pfSignerSign = (SignerSignType)GetProcAddress(hMssign32, "SignerSign");
								if (pfSignerSign) {
									SIGN_FILE_RESULT::SIGNING_FILE SF;
									BDL::EnterPasswordDlgInitParamW EPD;
									LPWSTR Password = nullptr;
									EPD.Caption = L"Введите пароль от сертификата";
									EPD.EditPasswordCaption = L"пароль от сертификата";
									EPD.Password = &Password;
									EPD.ToolTipCaption[0] = L"Показать пароль от сертификата";
									EPD.ToolTipCaption[1] = L"Скрыть пароль от сертификата";
									EPD.HasToolTip = true;
									EPD.hIconCaption = LoadIconW(PS.hInst, MAKEINTRESOURCEW(IDI_SIGNPROJECTTOOL));
									if (DialogBoxParamW(hBDL, MAKEINTRESOURCEW(IDDBDL_ENTERPASSWORD), NULL, BDL::EnterPasswordDlgProcW, (LPARAM)& EPD) == IDOK) {
										LoadCertificateResult LCR = LoadCertificate(hDlg, &PP.CertificateFile, false, Password);
										SecureZeroMemory(Password, EPD.PasswordSize);
										LocalFree(Password);
										if (LCR.CertificateIsLoaded) {
											if(!PS.ConsoleIsAlloced)PS.ConsoleIsAlloced = AllocConsole();
											else {
												FreeConsole();
												PS.ConsoleIsAlloced = AllocConsole();
											}
											if (PS.ConsoleIsAlloced == NULL)MessageError(L"Не удалось выделить консоль, отчёты о подписанных и неподписанных файлах отображаться не будут!", L"Ошибка выделения консоли!", hDlg);
											else {
												TSFINIT.hOutputConsole = GetStdHandle(STD_OUTPUT_HANDLE);
												//hInputConsole = GetStdHandle(STD_INPUT_HANDLE);
												TSFINIT.hErrorConsole = GetStdHandle(STD_ERROR_HANDLE);
												SetConsoleTitleW(L"Диагностическая информация о подписи файлов:");
												if ((TSFINIT.hOutputConsole == NULL) || (TSFINIT.hOutputConsole == INVALID_HANDLE_VALUE))MessageError(L"Не удалось получить дескриптор консоли для вывода информации о подписанных файлах!", L"Ошибка получения дескриптора консоли!", hDlg);
												//if ((hInputConsole == NULL) || (hInputConsole == INVALID_HANDLE_VALUE))MessageError(L"Не удалось получить дескриптор консоли для ввода!", L"Ошибка получения дескриптора консоли!", hDlg);
												if ((TSFINIT.hErrorConsole == NULL) || (TSFINIT.hErrorConsole == INVALID_HANDLE_VALUE))MessageError(L"Не удалось получить дескриптор консоли для вывода ошибок о неподписанных файлов!", L"Ошибка получения дескриптора консоли!", hDlg);
												if (SetConsoleCtrlHandler(ConsoleHandler, TRUE) == NULL)MessageError(L"Не удалось установить обработчик события консоли Ctrl+C", L"Ошибка установки обработчика!", hDlg);
												/*HWND hConsoleWindow = GetConsoleWindow();
												if (hConsoleWindow != NULL)DisableWindowClose(hConsoleWindow);
												else MessageError(L"Не удалось получить дескриптор окна консоли для отключения кнопки закрытия!", L"Ошибка получения дескриптора!", hDlg);*/
											}
											SF.SignCertificate = LCR.pCertContext;
											TSFINIT.hDlg = hDlg;
											TSFINIT.hStatusBar = hStatusBar;
											TSFINIT.pfSignerSign = pfSignerSign;
											TSFINIT.hCertStore = LCR.hCertStore;
											TSFINIT.SignCertificate = LCR.pCertContext;
											TSFINIT.ConsoleIsAlloced = PS.ConsoleIsAlloced;
											//закрытия дескриптора предыдущего потока, если он не был закрыт
											if (PS.hSignFilesThread != NULL)if(CloseHandle(PS.hSignFilesThread) == NULL)MessageError(L"Не удаллось закрыть дескриптор потока подписи файлов!", L"Ошибка закрытия дескриптора!", hDlg);
											PS.hSignFilesThread = CreateThread(NULL, NULL, SignFilesThreadProc, (LPVOID)& TSFINIT, NULL, &ThreadSigningFilesId);//создание потока для подписи файлов
											if (PS.hSignFilesThread == NULL) {
												MessageError(L"Не удалось создать поток для подписи файлов! Дальнейшая подпись файлов невозможна, попробуйте повторить попытку подписи.", L"Ошибка создания потока!", hDlg); 
												EnableWindow(hSign, TRUE);
												EnableWindow(hClear, TRUE);
												EnableWindow(hModifyFile, TRUE);
												EnableWindow(hDeleteFile, TRUE);
												EnableWindow(hAddFiles, TRUE);
												SetWindowTextW(hQuitCancel, L"Выход");
												SendMessageW(hStatusBar, SB_SETTEXTW, MAKEWORD(0, SBT_POPOUT), (LPARAM)L"Подпись не удалось начать");
											}
										}
										else MessageBoxW(hDlg, LCR.ErrorInfo.ErrorString.c_str(), LCR.ErrorInfo.ErrorCaption.c_str(), MB_OK | MB_ICONERROR);
									}

									
								}
								else MessageError(L"Не удалось получить адрес функции SignerSign из Mssign32.dll, дальнейшее подписание файлов не возможно!", L"Ошибка получения адресса функции!", NULL);
								//FreeLibrary(hMssign32);
							}
							else MessageError(L"не удалось загрузить библиотеку Mssign32.dll", L"Ошибка загрузки библиотеки!", NULL);
							FreeLibrary(hBDL);
						}
						else MessageError(L"Не удалось загрузить библиотеку BDL.dll!", L"Ошибка загрузки библиотеки!", hDlg);
						
					}
					else MessageError(L"Не удалось получить количество файлов в списке", L"Ошибка получения кол-ва файлов!", hDlg);//в случае ошибки получения кол-ва элементов
					//return (INT_PTR)TRUE;
					break;
				}
				case IDC_PAUSE_SIGNING: {//обработка кнопки приостановки подписи
					if (PS.hSignFilesThread != NULL) {
						DWORD result = WaitForSingleObject(PS.hSignFilesThread, 0);
						if (result != WAIT_OBJECT_0) {
							HWND hProgressForSigningFiles = GetDlgItem(hDlg, IDC_PROGRESS_SIGNING_FILES);
							// the thread handle is signaled - the thread has terminated
							if (ThreadForSigningFilesIsSuspend) {
								ResumeThread(PS.hSignFilesThread);
								ThreadForSigningFilesIsSuspend = FALSE;
								SetWindowTextW(hPauseSigning, L"Приостановить");
								if (hStatusBar != NULL)SendMessageW(hStatusBar, SB_SETTEXTW, MAKEWORD(0, SBT_POPOUT), (LPARAM)L"Идёт подпись файлов");
								SendMessageW(hProgressForSigningFiles, PBM_SETSTATE, PBST_NORMAL, 0);
							}
							else {
								SuspendThread(PS.hSignFilesThread);
								ThreadForSigningFilesIsSuspend = TRUE;
								SetWindowTextW(hPauseSigning, L"Продолжить");
								if (hStatusBar != NULL)SendMessageW(hStatusBar, SB_SETTEXTW, MAKEWORD(0, SBT_POPOUT), (LPARAM)L"Подпись приостановлена");
								SendMessageW(hProgressForSigningFiles, PBM_SETSTATE, PBST_PAUSED, 0);
							}
							break;
						}
					}
					else EnableWindow(hPauseSigning, FALSE);
					break; 
				}
				case IDCANCEL:{//обработка кнопки "Выход"/"Отмена"
					if (PS.hSignFilesThread != NULL) {
						DWORD result = WaitForSingleObject(PS.hSignFilesThread, 0);
						if (result != WAIT_OBJECT_0) {
							// the thread handle is signaled - the thread has terminated
							PostThreadMessageW(ThreadSigningFilesId, PARAMETERS_FOR_THREAD_SIGN_FILES::THREAD_MESSAGES::THM_CANCEL, 0, 0);
							if (ThreadForSigningFilesIsSuspend)ResumeThread(PS.hSignFilesThread);
							break;
						}
						if (CloseHandle(PS.hSignFilesThread) == NULL)MessageError(L"Не удаллось закрыть дескриптор потока подписи файлов!", L"Ошибка закрытия дескриптора!", hDlg);
						else PS.hSignFilesThread = NULL;
					}
				}
				case IDM_EXIT://обработка пункта меню "Выход" и кнопки закрытия окна
				case WM_CLOSE: {
					DestroyWindow(hDlg);
					return (INT_PTR)TRUE;
				}
				case ID_CLOSE_CONSOLE: {
					if ((PS.hSignFilesThread != NULL) && (PS.ConsoleIsAlloced == TRUE)) {
						if (WaitForSingleObject(PS.hSignFilesThread, 0) == WAIT_OBJECT_0) {
							if(FreeConsole() != NULL)PS.ConsoleIsAlloced = FALSE;
							else {
								DWORD ErrorFreeConsole = GetLastError();
								if (ErrorFreeConsole == ERROR_INVALID_PARAMETER) {
									MessageBoxW(hDlg, L"Не удалось выполнить освобождение консоли!\n Причина ошибки: консоль не была подключена к процессу.", L"Ошибка освобождения консоли!", MB_OK | MB_ICONERROR); 
									PS.ConsoleIsAlloced = FALSE;
								}
							}
							if (CloseHandle(PS.hSignFilesThread) == NULL)MessageError(L"Не удаллось закрыть дескриптор потока подписи файлов!", L"Ошибка закрытия дескриптора!", hDlg);
							else PS.hSignFilesThread = NULL;
						}
					}
					break;
				}
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
					};//
					WSTRINGARRAY OpenningFiles = OpenFiles(hDlg, cmf, sizeof(cmf)/sizeof(cmf[0]), L"Выберете файлы для цифровой подписи", L"Добавить...", true);
					size_t OpenningFilesSize = OpenningFiles.size();
					if (OpenningFilesSize > 0) {
						for (size_t i = 0; i < OpenningFilesSize; i++) {
							if (!StringExistInListBox(hListSelectedFilesForCertification, OpenningFiles[i])) {
								if (SendMessageW(hListSelectedFilesForCertification, LB_ADDSTRING, NULL, (LPARAM)OpenningFiles[i].c_str()) != LB_ERR) {
									UINT Temp = CalcBItemWidth(hListSelectedFilesForCertification, OpenningFiles[i].c_str());
									if (Temp > MaxStringWhidth)MaxStringWhidth = Temp;
								}
								else MessageError(L"Не удалось добавить файл!", L"Ошибка добавления файла!", hDlg);
							}
						}
						SendMessageW(hListSelectedFilesForCertification, LB_SETHORIZONTALEXTENT, MaxStringWhidth, 0);
						if (PP.CertificateFile != L"")EnableWindow(hSign, TRUE);
						EnableWindow(hModifyFile, TRUE);
						EnableWindow(hClear, TRUE);
					}
					break;
				}
				case IDC_DELETE_FILE:{//обработки кнопки "Удалить"
					LRESULT ItemsCount = SendMessageW(hListSelectedFilesForCertification, LB_GETCOUNT, NULL, NULL);// получение кол-ва элементов(файлов) в списке
					if (ItemsCount != LB_ERR) {//проверка на ошибку
						MaxStringWhidth = 0;//обнуление длинны прокрутки горизонтального скролбара ListBox
						for (LRESULT i = 0; i < ItemsCount; i++) {//цикл перебора строк 
							LRESULT ItemIsSelected = SendMessageW(hListSelectedFilesForCertification, LB_GETSEL, (WPARAM)i, NULL);//получение информации о том, выбран ли элемент
							if (ItemIsSelected > 0) {// если выбран
								if (SendMessageW(hListSelectedFilesForCertification, LB_DELETESTRING, (WPARAM)i, NULL) != LB_ERR) {// и если не ошибка, то удаляем его
									ItemsCount--; i--;// и уменьшаем итератор и кол-во элементов в списке
								}
								else MessageError(L"Не удалось удалить выбранный элемент из списка файлов!", L"Ошибка удаления выбранного элемента!", hDlg);// в случае ошибки
							}
							else if (ItemIsSelected == 0) {// если не выбран
								WCHARVECTOR ListBoxString;// здесь будет хранится полученная строка из ListBox
								LRESULT TextLen = SendMessageW(hListSelectedFilesForCertification, LB_GETTEXTLEN, (WPARAM)i, NULL);//получаем длинну строки
								if (TextLen != LB_ERR) {// если не ошибка
									ListBoxString.resize(TextLen+1);//инициализируем массив ListBoxString, к TextLen прибавляем 1, т.к. длинна, которую мы получили в предыдущем шаге не включает терминирующий ноль
									if (SendMessageW(hListSelectedFilesForCertification, LB_GETTEXT, (WPARAM)i, (LPARAM)ListBoxString.data()) != LB_ERR) {//получаем текст элемента, если он был получен успешно
										UINT Temp = CalcBItemWidth(hListSelectedFilesForCertification, ListBoxString.data());//пересчитываем длинну прокрутки горизонтального скролбара
										if (Temp > MaxStringWhidth) MaxStringWhidth = Temp;//если она больше максимальной, то присваиваем её максимальной
									}
									else MessageError(L"Не удалось получить текст невыбранного элемента!", L"Ошибка при получении текста невыбранного элемента!", hDlg);//в случае неудачного получения текста элемента
								}
								else MessageError(L"Не удалось узнать длинну текста невыбранного элемента!", L"Ошибка получения длинны невыбранного элемента!", hDlg);//в случае неудачного получения длинны строки
							}
							else if (ItemIsSelected == LB_ERR)MessageError(L"Не удалось узнать выбран ли элемент", L"Ошибка при распознавании выбранного элемента!", hDlg);//в случае ошибки проверки выбран ли элемент
						}
						if (ItemsCount == 0) {// если список стал пустым
							//отключение всех органов управления, для которых требуется наличие хотя бы одного файла в списке
							EnableWindow(hModifyFile, FALSE);
							EnableWindow(hSign, FALSE);
							EnableWindow(hClear, FALSE);
						}
						if (SendMessageW(hListSelectedFilesForCertification, LB_SETHORIZONTALEXTENT, MaxStringWhidth, 0) == LB_ERR)MessageError(L"Не удалось установить прокручиваемую область!", L"Ошибка установки прокручиваемой области!", hDlg);//реинициализируем горизонтальный скролбар ListBox новым значением длинны прокрутки
						EnableWindow(hDeleteFile, FALSE);
					}
					else MessageError(L"Не удалось получить количество элементов", L"Ошибка получения количества элементов!", hDlg);//в случае ошибки получения кол-ва элементов
					break;
				} 
				case IDC_MODIFY_FILE:{ //обработка кнопки "Изменить"
					
					break;
				}
				case IDC_CLEAR://обработка кнопки "Очистить
					if (SendMessageW(hListSelectedFilesForCertification, LB_RESETCONTENT, NULL, NULL) == LB_ERR)MessageError(L"Не удалось очистить список!", L"Ошибка очистки списка!", hDlg);
					MaxStringWhidth = 0;//обнуление длинны прокрутки горизонтального скролбара ListBox
					// "выключение" горизонтального скролбара
					if (SendMessageW(hListSelectedFilesForCertification, LB_SETHORIZONTALEXTENT, MaxStringWhidth, 0) == LB_ERR)MessageError(L"Не удалось установить прокручиваемую область!", L"Ошибка установки прокручиваемой области!", hDlg);
					//отключение всех органов управления, для которых требуется наличие хотя бы одного файла в списке
					EnableWindow(hDeleteFile, FALSE);
					EnableWindow(hModifyFile, FALSE);
					EnableWindow(hSign, FALSE);
					EnableWindow(hClear, FALSE);
					break;
				case IDC_CHECK_PERFORM_ACTIONS_AFTER_SIGNING: {
					if (IsDlgButtonChecked(hDlg, IDC_CHECK_PERFORM_ACTIONS_AFTER_SIGNING)) {
						INT_PTR ResultDialog = DialogBoxParamW(PS.hInst, MAKEINTRESOURCEW(IDD_SETTINGS_FOR_ACTIONS_PERFORMED_AFTER_SIGNING), hDlg, SettingsForActionsPerformedAfterSigningDlgProc, (LPARAM)&PP.CommandToExecuteAfterSigningFiles);
						if (HIWORD(ResultDialog) == IDOK) {
							PP.CommandToExecuteAfterSigningFiles = LOWORD(ResultDialog);
						}
					}
					else PP.CommandToExecuteAfterSigningFiles = 0; 
					break;
				}
				case IDM_SHOW_FILE_IN_EXPLORER: {
					LRESULT IndexCount = SendMessageW(hListSelectedFilesForCertification, LB_GETSELCOUNT, NULL, NULL);
					if (IndexCount != 0) {;
							UINTVECTOR SelectIndexs;
							SelectIndexs.resize(IndexCount);
							SendMessageW(hListSelectedFilesForCertification, LB_GETSELITEMS, IndexCount, (LPARAM)SelectIndexs.data());
							for (UINT i = 0; i < IndexCount; i++) {
								LRESULT FileNameLength = SendMessageW(hListSelectedFilesForCertification, LB_GETTEXTLEN, SelectIndexs[i], NULL);
								if (FileNameLength != LB_ERR) {
									WSTRING FileName;
									FileName.resize(FileNameLength + 1);
									if (SendMessageW(hListSelectedFilesForCertification, LB_GETTEXT, SelectIndexs[i], (LPARAM)FileName.data()) != LB_ERR) {
										FileName = L"/select, " + FileName;
										ShellExecuteW(NULL, NULL, L"explorer.exe", FileName.data(), NULL, SW_SHOWNORMAL);
									}
									else MessageError(L"Ошибка получения текста элемента в списке при обработке пункта меню \"Показать в проводнике\"", L"Ошибка при обработке пункта меню!", hDlg);
								}
								else MessageError(L"Ошибка при получении длинны текста элемента в списке при обработке пункта меню \"Показать в проводнике\"", L"Ошибка при обработке пункта меню!", hDlg);								
							}
					}
					break;
				}
				case IDM_CLEAR_SELECT_IN_LIST: {
					LRESULT CountSelectedElements = SendMessageW(hListSelectedFilesForCertification, LB_GETSELCOUNT, NULL, NULL);
					LRESULT CountElements = SendMessageW(hListSelectedFilesForCertification, LB_GETCOUNT, NULL, NULL);
					if (CountSelectedElements == CountElements)SendMessageW(hListSelectedFilesForCertification, LB_SETSEL, FALSE, -1);
					else {
						if (CountSelectedElements != 0) {
							UINTVECTOR SelectedIndexs;
							SelectedIndexs.resize(CountSelectedElements);
							if (SendMessageW(hListSelectedFilesForCertification, LB_GETSELITEMS, (WPARAM)CountSelectedElements, (LPARAM)SelectedIndexs.data()) != LB_ERR) {
								for (UINT i = 0; i < CountSelectedElements; i++) {
									if (SendMessageW(hListSelectedFilesForCertification, LB_SETSEL, FALSE, SelectedIndexs[i]) == LB_ERR)MessageError(L"Произошла ошибка при отмене выбора элемента в списке при обработке пункта меню  \"Очистить выбор\"", L"Ошибка при обработке пункта меню!", hDlg);
								}
							}
						}
					}
					EnableWindow(hDeleteFile, FALSE);
					break;
				}
				case IDM_SELECT_ALL_IN_LIST: {
					SendMessageW(hListSelectedFilesForCertification, LB_SETSEL, TRUE, -1);
					break;
				}
				
			}
	}
	return (INT_PTR)FALSE;
}

/*INT_PTR CALLBACK MainConsoleWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch(message){
		case WM_CLOSE:
			FreeConsole();
			return 0;
	}
	return PP.DefaultConsoleWindowProc(hWnd, message, wParam, lParam);
}*/



bool ThisStringIsProgrammArgument(WSTRING arg) {
	for (size_t i = 0; i < ProgrammSettings::CommandLineValidArguments.size(); i++)if (arg == ProgrammSettings::CommandLineValidArguments[i])return true;
	return false;
}

/*BOOL CALLBACK FindListBoxChildWindowProc(HWND hWnd, LPARAM lParam)
{
	HWND* hListBoxWndPrt = (HWND *)lParam;
	WCHARVECTOR ClassName;
	ClassName.resize(100);
	if (GetClassNameW(hWnd, ClassName.data(), 100) != 0) {
		WSTRING ClassNameString = ClassName.data();
		ClassName.clear();
		if (ClassNameString == L"listbox") {
			*hListBoxWndPrt = hWnd;
			return FALSE;
		}
	}
	return TRUE;
}*/
BOOL WINAPI ConsoleHandler(_In_ DWORD dwCtrlType)
{
	switch (dwCtrlType){
		case CTRL_C_EVENT:
			if (PS.hSignFilesThread != NULL) {
				if (WaitForSingleObject(PS.hSignFilesThread, 0) == WAIT_OBJECT_0) {
					if (FreeConsole() != NULL)PS.ConsoleIsAlloced = FALSE;
					else {
						DWORD ErrorFreeConsole = GetLastError();
						if (ErrorFreeConsole == ERROR_INVALID_PARAMETER) {
							MessageBoxW(PS.hRootWnd, L"Не удалось выполнить освобождение консоли!\n Причина ошибки: консоль не была подключена к процессу.", L"Ошибка освобождения консоли!", MB_OK | MB_ICONERROR);
							PS.ConsoleIsAlloced = FALSE;
						}
					}
					if (CloseHandle(PS.hSignFilesThread) == NULL)MessageError(L"Не удаллось закрыть дескриптор потока подписи файлов!", L"Ошибка закрытия дескриптора!", PS.hRootWnd);
					else PS.hSignFilesThread = NULL;
				}
			}
			else FreeConsole();
			return TRUE;
	}
	return FALSE;
}


bool CheckBit(unsigned word, unsigned bit) {// функция проверяет установлен ли бит под определённым номером
	return (word & (1u << bit)) != 0;
}

void ProgrammSettings::LoadProgrammSettingsOfRegistry(unsigned sfLoad) {//данная функция предназначена для загрузки параметров программы из реестра Windows
	HKEY hMainRootKey = NULL;
	LSTATUS RegOpenKeyResult = RegOpenKeyExW(HKEY_CURRENT_USER, ROOT_REGISTRY_KEY_SIGNPROJECT_TOOL, 0, KEY_READ, &hMainRootKey);
	if (RegOpenKeyResult == ERROR_SUCCESS) {
		//Блок получения параметра CertificateFile
		if (sfLoad & ProgrammSettings::REGISTRY_QUERY::SIGN_CERTIFICATE_FILE_NAME) {
			DWORD ValueType = NULL, ValueSize = 1;
			WCHARVECTOR CertificateFileFromRegistry;
			CertificateFileFromRegistry.resize(1);
		QueryValueCertificateFile:
			LSTATUS RegQueryValueResult = RegQueryValueExW(hMainRootKey, L"CertificateFile", NULL, &ValueType, (LPBYTE)CertificateFileFromRegistry.data(), &ValueSize);
			if (RegQueryValueResult == ERROR_MORE_DATA) {
				if (ValueType == REG_EXPAND_SZ) {
					CertificateFileFromRegistry.resize(ValueSize / sizeof(WCHAR));
					goto QueryValueCertificateFile;
				}
				else {
					MessageBoxW(PS.hRootWnd, L"Данные в реестре по пути HKEY_CURRENT_USER\\" ROOT_REGISTRY_KEY_SIGNPROJECT_TOOL " повреждены. Параметр CertificateFile имеет неверный тип.", L"Неверный тип параметра в реестре", MB_OK | MB_ICONERROR);
					int ResultQuestion = MessageBoxW(PS.hRootWnd, L"Восстановить повреждённый параметр?", L"Требуется ваше вмешательство", MB_YESNO | MB_ICONQUESTION);
					switch (ResultQuestion) {
					case IDYES:
						SaveProgrammSettingsInRegistry(SIGN_CERTIFICATE_FILE_NAME);
						break;
					case IDNO:
					case IDCANCEL:
						break;
					}
				}

			}
			else if (RegQueryValueResult == ERROR_FILE_NOT_FOUND) {
				MessageBoxW(PS.hRootWnd, L"Ошибка получения значения параметра CertificateFile по пути HKEY_CURRENT_USER\\" ROOT_REGISTRY_KEY_SIGNPROJECT_TOOL ". Такого параметра не сущевтует.", L"Ошибка получения значения", MB_OK | MB_ICONERROR);
				int ResultQuestion = MessageBoxW(PS.hRootWnd, L"Создать отсуствующий параметр?", L"Требуется ваше вмешательство", MB_YESNO | MB_ICONQUESTION);
				switch (ResultQuestion) {
				case IDYES:
					SaveProgrammSettingsInRegistry(SIGN_CERTIFICATE_FILE_NAME);
					break;
				case IDNO:
				case IDCANCEL:
					break;
				}
			}
			else if (RegQueryValueResult == ERROR_SUCCESS) {
				if (CertificateFileFromRegistry.size() > 1) {
					if (FileReadebleExists(CertificateFileFromRegistry.data()))this->CertificateFile = CertificateFileFromRegistry.data();
					else {
						WSTRING FileHref = CertificateFileFromRegistry.data();
						WSTRING ErrorText = L"Не удалось открыть файл сертификата по пути \"" + FileHref + "\" используя параметр CertificateFile по пути в реестре \"HKEY_CURRENT_USER\\" ROOT_REGISTRY_KEY_SIGNPROJECT_TOOL "\" в качестве пути к файлу. Вам придётся выбрать сертификат в ручную.";
						MessageBoxW(PS.hRootWnd, ErrorText.c_str(), L"Ошибка открытия файла!", MB_OK | MB_ICONERROR);
					}
				}
			}
			else MessageError(L"Ошибка получения значения параметра CertificateFile из реестра по пути HKEY_CURRENT_USER\\" ROOT_REGISTRY_KEY_SIGNPROJECT_TOOL ".", L"Ошибка получения параметра!", PS.hRootWnd, RegQueryValueResult);
		}
		//Блок получения параметра AlgHash
		if (sfLoad & ProgrammSettings::REGISTRY_QUERY::ALG_HASH) {
			DWORD AlgHash = NULL;
			DWORD ValueType = NULL, ValueSize = sizeof(DWORD);
			LSTATUS RegQueryValueResult = RegQueryValueExW(hMainRootKey, L"AlgHash", NULL, &ValueType, (LPBYTE)& AlgHash, &ValueSize);
			if (RegQueryValueResult == ERROR_SUCCESS) {
				if (ValueType == REG_DWORD)this->HashAlgorithmId = AlgHash;
				else {
					MessageBoxW(PS.hRootWnd, L"Данные в реестре по пути HKEY_CURRENT_USER\\" ROOT_REGISTRY_KEY_SIGNPROJECT_TOOL " повреждены. Параметр AlgHash имеет неверный тип.", L"Неверный тип параметра в реестре", MB_OK | MB_ICONERROR);
					int ResultQuestion = MessageBoxW(PS.hRootWnd, L"Восстановить повреждённый параметр?", L"Требуется ваше вмешательство", MB_YESNO | MB_ICONQUESTION);
					switch (ResultQuestion) {
					case IDYES:
						SaveProgrammSettingsInRegistry(ALG_HASH);
						break;
					case IDNO:
					case IDCANCEL:
						break;
					}
				}
			}
			else if (RegQueryValueResult == ERROR_FILE_NOT_FOUND) {
				MessageBoxW(PS.hRootWnd, L"Ошибка получения значения параметра AlgHash из реестра по пути HKEY_CURRENT_USER\\" ROOT_REGISTRY_KEY_SIGNPROJECT_TOOL " . Такого параметра не сущевтует.", L"Ошибка получения значения", MB_OK | MB_ICONERROR);
				int ResultQuestion = MessageBoxW(PS.hRootWnd, L"Создать отсуствующий параметр?", L"Требуется ваше вмешательство", MB_YESNO | MB_ICONQUESTION);
				switch (ResultQuestion) {
				case IDYES:
					SaveProgrammSettingsInRegistry(ALG_HASH);
					break;
				case IDNO:
				case IDCANCEL:
					break;
				}
			}
			else MessageError(L"Ошибка получения значения параметра AlgHash из реестра по пути HKEY_CURRENT_USER\\" ROOT_REGISTRY_KEY_SIGNPROJECT_TOOL ".", L"Ошибка получения параметра!", PS.hRootWnd, RegQueryValueResult);
		}
	}
	else {
		MessageError(L"Ошибка открытия ключа реестра HKEY_CURRENT_USER\\" ROOT_REGISTRY_KEY_SIGNPROJECT_TOOL ".", L"Ошибка открытия ключа", PS.hRootWnd, RegOpenKeyResult);
		MessageBoxW(PS.hRootWnd, L"Будут загружены стандартные настройки программы", L"Информация", MB_OK | MB_ICONINFORMATION);
	}
}
//Данная функция сохраняет параметры программы в реестр, на вход она принимает комбинацию битовых флагов 
void ProgrammSettings::SaveProgrammSettingsInRegistry(unsigned sfSave) {//данная функция преназначена для сохранения параметров программы в реестр Windows
	HKEY hMainRootKey = NULL;
	DWORD dwDisposition = NULL, dwNull = 0;
	WCHAR TransactionDes[] = L"Транзакция для работы с ключом SignProjectTool";
	HANDLE hTransaction = CreateTransaction(NULL, 0, TRANSACTION_DO_NOT_PROMOTE, (DWORD)& dwNull, (DWORD)& dwNull, INFINITE, TransactionDes);
	if (hTransaction != INVALID_HANDLE_VALUE) {
		LSTATUS RegCreateKeyTransactedResult = RegCreateKeyTransactedW(HKEY_CURRENT_USER, ROOT_REGISTRY_KEY_SIGNPROJECT_TOOL, 0, NULL, REG_OPTION_VOLATILE, KEY_WRITE, NULL, &hMainRootKey, &dwDisposition, hTransaction, NULL);
		if (RegCreateKeyTransactedResult == ERROR_SUCCESS) {
			switch (dwDisposition) {
			case REG_CREATED_NEW_KEY:
			case REG_OPENED_EXISTING_KEY:
				if (sfSave != NULL) {
					if (sfSave & ProgrammSettings::REGISTRY_QUERY::SIGN_CERTIFICATE_FILE_NAME) {
						LSTATUS RegSetValueExResult = RegSetValueExW(hMainRootKey, L"CertificateFile", 0, REG_EXPAND_SZ, (BYTE*)this->CertificateFile.c_str(), (this->CertificateFile.size() + 1) * sizeof(WCHAR));
						if (RegSetValueExResult != ERROR_SUCCESS)MessageError(L"Не удалось создать параметр CertificateFile в реестре по пути HKEY_CURRENT_USER\\" ROOT_REGISTRY_KEY_SIGNPROJECT_TOOL, L"Ошибка создания параметра!", PS.hRootWnd, RegSetValueExResult);
					}
					if (sfSave & ProgrammSettings::REGISTRY_QUERY::ALG_HASH) {
						LSTATUS RegSetValueExResult = RegSetValueExW(hMainRootKey, L"AlgHash", 0, REG_DWORD, (BYTE*)& HashAlgorithmId, sizeof(HashAlgorithmId));
						if (RegSetValueExResult != ERROR_SUCCESS)MessageError(L"Не удалось создать параметр CertificateFile в реестре по пути HKEY_CURRENT_USER\\" ROOT_REGISTRY_KEY_SIGNPROJECT_TOOL, L"Ошибка создания параметра!", PS.hRootWnd, RegSetValueExResult);
					}
				}
				else {
					LSTATUS RegSetValueExResult = RegSetValueExW(hMainRootKey, L"CertificateFile", 0, REG_EXPAND_SZ, (BYTE*)this->CertificateFile.c_str(), (this->CertificateFile.size() + 1) * sizeof(WCHAR));
					if (RegSetValueExResult != ERROR_SUCCESS)MessageError(L"Не удалось создать параметр CertificateFile в реестре по пути HKEY_CURRENT_USER\\" ROOT_REGISTRY_KEY_SIGNPROJECT_TOOL, L"Ошибка создания параметра!", PS.hRootWnd, RegSetValueExResult);
					RegSetValueExResult = RegSetValueExW(hMainRootKey, L"AlgHash", 0, REG_DWORD, (BYTE*)& HashAlgorithmId, sizeof(HashAlgorithmId));
					if (RegSetValueExResult != ERROR_SUCCESS)MessageError(L"Не удалось создать параметр CertificateFile в реестре по пути HKEY_CURRENT_USER\\" ROOT_REGISTRY_KEY_SIGNPROJECT_TOOL, L"Ошибка создания параметра!", PS.hRootWnd, RegSetValueExResult);
				}
				CommitTransaction(hTransaction);
				CloseHandle(hTransaction);
			}
		}
		else {
			MessageError(L"Не удалось создать/открыть ключ реестра по пути HKEY_CURRENT_USER\\" ROOT_REGISTRY_KEY_SIGNPROJECT_TOOL, L"Ошибка создания/открытия ключа", PS.hRootWnd, RegCreateKeyTransactedResult);
			CloseHandle(hTransaction);
		}
	}
	else MessageError(L"Не удалось создать транзакцию для создания основной ветки реестра программы", L"Ошибка создания транзакции!", PS.hRootWnd);
}