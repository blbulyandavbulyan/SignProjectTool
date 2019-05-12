// SignProjectTool.cpp : Определяет точку входа для приложения.
//

#include "stdafx.h"
#include "SignProjectTool.h"
#define MAX_LOADSTRING 100
//структура с основными параметрами программы
using namespace WWS;
struct ProgrammSettings {
	HINSTANCE hInst;//экземпляр приложения
	WSTRINGARRAY FilesForCertification;//список файлов для подписи
	WSTRING CertificateFile;// имя файла сертификата для подписи им
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
struct LoadCertificateResult {//данную структуру возвращает функция LoadCertificate
	bool CertificateIsLoaded = false;//сертификат был загружен
	PCCERT_CONTEXT pCertContext = nullptr;//структура, содержащая загруженный сертификат
	HCERTSTORE hCertStore = nullptr;
	WWS_ERROR_REPORT_STRUCTW ErrorInfo;//в случае если сертификат не был загружен здесь содержится информация об ошибке
};
struct SIGN_FILE_RESULT {//данную структуру возвращает функция SignFile
	struct SIGNING_FILE {//данная структура передаётся функции  SignFile в качестве параметра
		PCCERT_CONTEXT SignCertificate = nullptr;
		LPCWSTR SigningFileName = nullptr;//имя подписываемого файла
		ALG_ID HashAlgorithmId = PP.HashAlgorithmId;// алгоритм хеширования для цифровой подписи
		DWORD dwAttrChoice = 0;//требуется ли использовать pAttrAuthcode
		SIGNER_ATTR_AUTHCODE *pAttrAuthcode = nullptr;//дополнительные параметры для подписи
	};
	typedef const SIGNING_FILE *PCSIGNING_FILE;
	bool FileIsSigned = false;//файл был подписан
	bool InitializationFailed = false;//была ли ошибка инициализации функции SignFile
	WWS_ERROR_REPORT_STRUCTW ErrorInfo;//данный член будет заполнен в том случае, если не удастся инициализировать функцию SIGN_FILES_RESULT
};
typedef HLOCAL(*PFreeAlocatedBuffer)(HLOCAL Buffer);
// Отправить объявления функций, включенных в этот модуль кода:
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK StartConfigurationDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AddFilesForCertificationDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
UINT CalcBItemWidth(HWND hLB, LPCTSTR Text);
WSTRINGARRAY OpenFiles(HWND hWnd, COMDLG_FILTERSPEC *cmf, UINT cmfSize, LPCWSTR TitleFileDialog, LPCWSTR OKButtonTitle, bool Multiselect);
SIGN_FILE_RESULT SignFile(SIGN_FILE_RESULT::PCSIGNING_FILE SF, SignerSignType pfSignerSign);
LoadCertificateResult LoadCertificate(HWND hWnd, const WSTRING *CertificateHref, bool LoadCertificateFromCertStore, LPCWSTR password);
bool ThisStringIsProgrammArgument(WSTRING arg);
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);
	//Parse command line
	PP.hInst = hInstance;
	bool NoGraphicsMode = false;
	WSTRINGARRAY CommandArray = BreakAStringIntoAnArrayOfStringsByCharacter(lpCmdLine, L' ');
	size_t CommandArraySize = CommandArray.size();
	for (size_t i = 0; i < CommandArraySize; i++) {
		if (CommandArray[i] == L"--about") {
			DialogBoxW(PP.hInst, MAKEINTRESOURCEW(IDD_ABOUTBOX), NULL, About);
			return NULL;
		}
		else if (CommandArray[i] == L"--startup-config") {
			DialogBoxW(PP.hInst, MAKEINTRESOURCEW(IDD_START_CONFIGURATION), NULL, StartConfigurationDlgProc);
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
				MessageBoxW(NULL, L"Недостаточно аргументов командной строки, возможно вы забыли указать список входных файлов!", L"Ошибка аргументов командной строки!", MB_ICONERROR | MB_OK);
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
	WSTRING FileHref = L"C:\\Users\\Давид\\source\\repos\\certs\\Blbulyan Software.pfx", FileForSigning = L"C:\\Users\\Давид\\Desktop\\TestedAplication.exe";
	HMODULE hMssign32 = LoadLibrary(L"Mssign32.dll"), hBDL = LoadLibrary(L"BDL.dll");
	if (hBDL != NULL) {
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
				EPD.hIconCaption = LoadIcon(PP.hInst, MAKEINTRESOURCE(IDI_SIGNPROJECTTOOL));
				DialogBoxParamW(hBDL, MAKEINTRESOURCE(IDDBDL_ENTERPASSWORD), NULL, BDL::EnterPasswordDlgProcW, (LPARAM)& EPD);
				LoadCertificateResult LCR = LoadCertificate(NULL, &FileHref, false, Password);
				SecureZeroMemory(Password, EPD.PasswordSize);
				LocalFree(Password);
				if (LCR.CertificateIsLoaded) {
					SF.SignCertificate = LCR.pCertContext;
					SF.SigningFileName = FileForSigning.c_str();
					SIGN_FILE_RESULT result = SignFile(&SF, pfSignerSign);
					if (result.InitializationFailed) {
						MessageBoxW(NULL, L"Ошибка инициализации функции SignFile, дальнейшую подпись файлов продолжить невозможно!", L"Ошибка инициализации функции SignFile", MB_OK | MB_ICONERROR);
						MessageBoxW(NULL, result.ErrorInfo.ErrorString.c_str(), result.ErrorInfo.ErrorCaption.c_str(), MB_OK | MB_ICONERROR);
					}
					if (!result.FileIsSigned) {
						DWORD CountWriteChars = 0;
						result.ErrorInfo.ErrorCaption += L"\n";
						result.ErrorInfo.ErrorString += L"\n";
						WSTRING OutputString = result.ErrorInfo.ErrorCaption + result.ErrorInfo.ErrorString;
						OutputDebugStringW(OutputString.c_str());
					}
					else {
						DWORD CountWriteChars = 0;
						WSTRING OutputSigningFileInfo = L"Файл \"";
						OutputSigningFileInfo += FileForSigning.c_str();
						OutputSigningFileInfo += L"\" был успешно подписа\n";
						OutputDebugStringW(OutputSigningFileInfo.c_str());
					}
					CertFreeCertificateContext(LCR.pCertContext);
					CertCloseStore(LCR.hCertStore, CERT_CLOSE_STORE_FORCE_FLAG);
				}

			}
			else MessageError(L"Не удалось получить адрес функции SignerSign из Mssign32.dll, дальнейшее подписание файлов не возможно!", L"Ошибка получения адресса функции!", NULL);
			FreeLibrary(hMssign32);
		}
		else MessageError(L"не удалось загрузить библиотеку Mssign32.dll", L"Ошибка загрузки библиотеки!", NULL);
		FreeLibrary(hBDL);
	}
	else {
		MessageError(L"не удалось загрузить библиотеку BDL.dll!", L"Ошибка загрузки библиотеки!", NULL);
		return FALSE;
	}
	
	return NULL;
}

// Обработчик сообщений для окна "О программе".
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
		case WM_INITDIALOG:{
			HICON hDialogIcon = LoadIconW(PP.hInst, MAKEINTRESOURCEW(IDI_SIGNPROJECTTOOL));
			if (hDialogIcon == NULL)MessageError(L"Не удалось загрузить иконку для текущего диалога!", L"Ошибка загрузки иконки!", hDlg);
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
INT_PTR CALLBACK StartConfigurationDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
	HWND hCertStoreName = GetDlgItem(hDlg, IDC_CERT_STORE_NAME), hOpenFile = GetDlgItem(hDlg, IDC_OPEN_FILE), hCertFile = GetDlgItem(hDlg, IDC_CERT_FILE), hCertStore = GetDlgItem(hDlg, IDC_CERT_STORE);
	static WSTRING CertificateFileName;
	switch(message){
		case WM_INITDIALOG:{
			CertificateFileName = PP.CertificateFile;
			HICON hDialogIcon = LoadIconW(PP.hInst, MAKEINTRESOURCEW(IDI_SIGNPROJECTTOOL));
			if (hDialogIcon == NULL)MessageError(L"Не удалось загрузить иконку для текущего диалога!", L"Ошибка загрузки иконки!", hDlg);
			else {
				SendMessageW(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hDialogIcon);
				SendMessageW(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hDialogIcon);
			}
			SendMessageW(hCertStore, BM_SETCHECK, (WPARAM)BST_CHECKED, NULL);
			//начало операции удаления ненужного пункта меню
			HMENU hMainMenu = GetMenu(hDlg);//получения меню диалогового окна
			if (hMainMenu != NULL) {//если прошло успешно
				if (RemoveMenu(hMainMenu, IDM_STARTUPCONFIG, MF_BYCOMMAND) == NULL) {//то удаляем, если удаление завершилось неудачей
					MessageError(L"Не удалось удалить пункт меню!", L"Ошибка удаления пункта меню!", NULL);//выводим соотвествующую ошибку
					//начало процесса отключения пункта меню
					MENUITEMINFOW mi;//струтура с информацией о пунтке меню
					ZeroMemory(&mi, sizeof(MENUITEMINFOW));//обнуляем струткуру
					mi.cbSize = sizeof(MENUITEMINFOW);//размер данной струтуры
					mi.fState = MFS_DISABLED;//свойство для отключения пунтка меню
					mi.fMask = MIIM_STATE;//чтобы действовала группа свойств (в которой находится MFS_DISABLED)
					if (SetMenuItemInfoW(hMainMenu, IDM_STARTUPCONFIG, FALSE, &mi) == NULL)MessageError(L"Не удалось установить информацию о пункте меню!", L"Ошибка установки информации о пункте меню!", NULL);//отключаем, если неудача, то выводим ошибку
				}
			}
			else MessageError(L"Не удалось получить меню диалога!", L"Ошибка получения меню диалога!", NULL);
			EnableWindow(hOpenFile, FALSE);
			return (INT_PTR)TRUE;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDM_ABOUT:
					DialogBoxW(PP.hInst, MAKEINTRESOURCEW(IDD_ABOUTBOX), hDlg, About);
					break;
				case IDM_EXIT:
					EndDialog(hDlg, IDM_EXIT);
					break;
				case IDOK:
					PP.CertificateFile = CertificateFileName;
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
					COMDLG_FILTERSPEC cmf[1] = {//массив с фильтрами
						//фильтр для *.exe файлов
						{
							L"*.pfx файлы",
							L"*.pfx"
						}
					};
					WSTRINGARRAY CertificateFile = OpenFiles(hDlg, cmf, sizeof(cmf) / sizeof(COMDLG_FILTERSPEC), L"Загрузите сертификат для подписи", L"Загрузить", false);
					if (CertificateFile.size() > 0) {
						CertificateFileName = CertificateFile[0];
					}
					//break;
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
			HICON hDialogIcon = LoadIconW(PP.hInst, MAKEINTRESOURCEW(IDI_SIGNPROJECTTOOL));//загрузка иконки диалога из ресурсов
			if (hDialogIcon == NULL)MessageError(L"Не удалось загрузить иконку для текущего диалога!", L"Ошибка загрузки иконки!", hDlg);
			else {
				//установка иконки диалога
				SendMessageW(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hDialogIcon);
				SendMessageW(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hDialogIcon);
			}
			return (INT_PTR)TRUE;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDM_STARTUPCONFIG:
					DialogBoxW(PP.hInst, MAKEINTRESOURCEW(IDD_START_CONFIGURATION), NULL, StartConfigurationDlgProc);
					break;
				case IDM_ABOUT: //обработка пункта меню "О программе"
					DialogBoxW(PP.hInst, MAKEINTRESOURCEW(IDD_ABOUTBOX), hDlg, About);
					break;
				case IDC_SIGN:{//обработка кнопки "Подписать"
					LRESULT ItemsCount = SendMessageW(hListSelectedFilesForCertification, LB_GETCOUNT, NULL, NULL);// получение кол-ва элементов(файлов) в списке
					if (ItemsCount != LB_ERR) {
						BOOL ConsoleIsAlloced = AllocConsole();
						HANDLE hOutputConsole = nullptr, hInputConsole = nullptr, hErrorConsole = nullptr;
						if (ConsoleIsAlloced == NULL)MessageError(L"Не удалось выделить консоль, отчёты о подписанных и неподписанных файлах отображаться не будут!", L"Ошибка выделения консоли!", hDlg);
						else {
							hOutputConsole = GetStdHandle(STD_OUTPUT_HANDLE);
							hInputConsole = GetStdHandle(STD_INPUT_HANDLE);
							hErrorConsole = GetStdHandle(STD_ERROR_HANDLE);
							if ((hOutputConsole == NULL) || (hOutputConsole == INVALID_HANDLE_VALUE))MessageError(L"Не удалось получить дескриптор консоли для вывода информации о подписанных файлах!", L"Ошибка получения дескриптора консоли!", hDlg);
							if ((hInputConsole == NULL) || (hInputConsole == INVALID_HANDLE_VALUE))MessageError(L"Не удалось получить дескриптор консоли для ввода!", L"Ошибка получения дескриптора консоли!", hDlg);
							if ((hErrorConsole == NULL) || (hErrorConsole == INVALID_HANDLE_VALUE))MessageError(L"Не удалось получить дескриптор консоли для вывода ошибок о неподписанных файлов!", L"Ошибка получения дескриптора консоли!", hDlg);
						}
						HMODULE hBDL = LoadLibraryW(L"BDL.dll"), hMssign32 = LoadLibrary(L"Mssign32.dll");
						if (hBDL) {
							if (hMssign32 != NULL) {
								SignerSignType pfSignerSign = (SignerSignType)GetProcAddress(hMssign32, "SignerSign");
								if (pfSignerSign) {
									HWND hProgressForSigningFiles = GetDlgItem(hDlg, IDC_PORGRESS_SIGNING_FILES);
									EnableWindow(hProgressForSigningFiles, TRUE);
									SIGN_FILE_RESULT::SIGNING_FILE SF;
									BDL::EnterPasswordDlgInitParamW EPD;
									LPWSTR Password = nullptr;
									EPD.Caption = L"Введите пароль от сертификата";
									EPD.EditPasswordCaption = L"пароль от сертификата";
									EPD.Password = &Password;
									EPD.ToolTipCaption[0] = L"Показать пароль от сертификата";
									EPD.ToolTipCaption[1] = L"Скрыть пароль от сертификата";
									EPD.HasToolTip = true;
									EPD.hIconCaption = LoadIconW(PP.hInst, MAKEINTRESOURCEW(IDI_SIGNPROJECTTOOL));
									DialogBoxParamW(hBDL, MAKEINTRESOURCE(IDDBDL_ENTERPASSWORD), NULL, BDL::EnterPasswordDlgProcW, (LPARAM)& EPD);
									LoadCertificateResult LCR = LoadCertificate(hDlg, &PP.CertificateFile, false, Password);
									SecureZeroMemory(Password, EPD.PasswordSize);
									LocalFree(Password);
									if (LCR.CertificateIsLoaded) {
										SF.SignCertificate = LCR.pCertContext;
										SendMessageW(hProgressForSigningFiles, PBM_SETRANGE32, 0, ItemsCount);
										SendMessageW(hProgressForSigningFiles, PBM_SETSTEP, (WPARAM)1, NULL);
										for (LRESULT i = 0; i < ItemsCount; i++) {//цикл перебора строк 
											WCHARVECTOR FileForSigning;// здесь будет хранится полученная строка из ListBox
											LRESULT TextLen = SendMessageW(hListSelectedFilesForCertification, LB_GETTEXTLEN, (WPARAM)i, NULL);//получаем длинну строки
											if (TextLen != LB_ERR) {// если не ошибка
												FileForSigning.resize(TextLen + 1);//инициализируем массив ListBoxString, к TextLen прибавляем 1, т.к. длинна, которую мы получили в предыдущем шаге не включает терминирующий ноль
												if (SendMessageW(hListSelectedFilesForCertification, LB_GETTEXT, (WPARAM)i, (LPARAM)FileForSigning.data()) != LB_ERR) {//получаем текст элемента, если он был получен успешно
													SF.SigningFileName = FileForSigning.data();
													SIGN_FILE_RESULT result = SignFile(&SF, pfSignerSign);
													if (result.InitializationFailed) {
														MessageBoxW(hDlg, L"Ошибка инициализации функции SignFile, дальнейшую подпись файлов продолжить невозможно!", L"Ошибка инициализации функции SignFile", MB_OK | MB_ICONERROR);
														MessageBoxW(hDlg, result.ErrorInfo.ErrorString.c_str(), result.ErrorInfo.ErrorCaption.c_str(), MB_OK | MB_ICONERROR);
														if (ConsoleIsAlloced)FreeConsole();
														break;
													}
													if (!result.FileIsSigned) {
														DWORD CountWriteChars = 0;
														result.ErrorInfo.ErrorCaption += L"\n";
														result.ErrorInfo.ErrorString += L"\n";
														if ((hErrorConsole != NULL) && (hErrorConsole != INVALID_HANDLE_VALUE)) {
															WriteConsoleW(hErrorConsole, result.ErrorInfo.ErrorCaption.c_str(), result.ErrorInfo.ErrorCaption.size(), &CountWriteChars, NULL);
															WriteConsoleW(hErrorConsole, result.ErrorInfo.ErrorString.c_str(), result.ErrorInfo.ErrorString.size(), &CountWriteChars, NULL);
														}
														#ifdef _DEBUG
														else {
															WSTRING OutputString = result.ErrorInfo.ErrorCaption + result.ErrorInfo.ErrorString;
															OutputDebugStringW(OutputString.c_str());
														}
														#endif
													}
													else {
														DWORD CountWriteChars = 0;
														WSTRING OutputSigningFileInfo = L"Файл \"";
														OutputSigningFileInfo += FileForSigning.data();
														OutputSigningFileInfo += L"\" был успешно подписа\n";
														if ((hOutputConsole != NULL) && (hOutputConsole != INVALID_HANDLE_VALUE))WriteConsoleW(hOutputConsole, OutputSigningFileInfo.c_str(), OutputSigningFileInfo.size(), &CountWriteChars, NULL);
														#ifdef _DEBUG
														else OutputDebugStringW(OutputSigningFileInfo.c_str());
														#endif
													}
													SendMessageW(hProgressForSigningFiles, PBM_STEPIT, 0, 0);
												}
												else {
													//В случае ошибки получения имени файла
													DWORD CountWriteChars = 0;
													WSTRING OutputSigningFileInfo = L"Не получить имя файла под индексом ";
													OutputSigningFileInfo += std::to_wstring(i);
													OutputSigningFileInfo += L" в списке, файл не был подписан\n";
													if ((hErrorConsole != NULL) && (hErrorConsole != INVALID_HANDLE_VALUE))WriteConsoleW(hErrorConsole, OutputSigningFileInfo.c_str(), OutputSigningFileInfo.size(), &CountWriteChars, NULL);
													#ifdef _DEBUG
													else OutputDebugStringW(OutputSigningFileInfo.c_str());
													#endif
												}
											}
											else {
												//В случае ошибки получения длинны
												DWORD CountWriteChars = 0;
												WSTRING OutputSigningFileInfo = L"Не получить длинну имени файла под индексом ";
												OutputSigningFileInfo += std::to_wstring(i);
												OutputSigningFileInfo += L" в списке, файл не был подписан\n";
												if ((hErrorConsole != NULL) && (hErrorConsole != INVALID_HANDLE_VALUE))WriteConsoleW(hErrorConsole, OutputSigningFileInfo.c_str(), OutputSigningFileInfo.size(), &CountWriteChars, NULL);
												#ifdef _DEBUG
												else OutputDebugStringW(OutputSigningFileInfo.c_str());
												#endif
											}
										}
										EnableWindow(hProgressForSigningFiles, FALSE);
										if (ConsoleIsAlloced)FreeConsole();
										CertFreeCertificateContext(LCR.pCertContext);
										CertCloseStore(LCR.hCertStore, CERT_CLOSE_STORE_FORCE_FLAG);
									}
								}
								else MessageError(L"Не удалось получить адрес функции SignerSign из Mssign32.dll, дальнейшее подписание файлов не возможно!", L"Ошибка получения адресса функции!", NULL);
								FreeLibrary(hMssign32);
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
							else MessageError(L"Не удалось добавить файл!", L"Ошибка добавления файла!", hDlg);
						}
						SendMessageW(hListSelectedFilesForCertification, LB_SETHORIZONTALEXTENT, MaxStringWhidth, 0);
						EnableWindow(hDeleteFile, TRUE);
						EnableWindow(hModifyFile, TRUE);
						if (PP.CertificateFile != L"")EnableWindow(hSign, TRUE);
						EnableWindow(hClear, TRUE);
					}
					break;
				}
				case IDC_DELETE_FILE:{//обработки кнопки "Удалить"
					LRESULT ItemsCount = SendMessageW(hListSelectedFilesForCertification, LB_GETCOUNT, NULL, NULL);// получение кол-ва элементов(файлов) в списке
					//данный код предназначен для точной проверки что всё работает правильно(ВНИМАНИЕ ЭТО ОТЛАДОЧНЫЙ КОД, В КОНЕЧНОМ ПРОЕКТЕ ОН ПРИСУТСВОВАТЬ НЕ БУДЕТ, ДЛЯ ТОГО ЧТО БЫ ЕГО НЕ БЫЛО В СОБРАННОЙ ПРОГРАММЕ СМЕНИТЕ КОНФИГУРАЦИЯ С Debug на Release)
					#ifdef _DEBUG
					OutputDebugStringW(L"Этап до удаления: \n");
					WSTRINGARRAY ListBoxItems, ListBoxSelectedItems;
					for (LRESULT i = 0; i < ItemsCount; i++) {
						LRESULT ItemIsSelected = SendMessageW(hListSelectedFilesForCertification, LB_GETSEL, (WPARAM)i, NULL);//получение информации о том, выбран ли элемент
						if (ItemIsSelected > 0) {// если выбран
							WCHARVECTOR ListBoxString;// здесь будет хранится полученная строка из ListBox
							LRESULT TextLen = SendMessageW(hListSelectedFilesForCertification, LB_GETTEXTLEN, (WPARAM)i, NULL);//получаем длинну строки
							if (TextLen != LB_ERR) {// если не ошибка
								ListBoxString.resize(TextLen + 1);//инициализируем массив ListBoxString, к TextLen прибавляем 1, т.к. длинна, которую мы получили в предыдущем шаге не включает терминирующий ноль
								if (SendMessageW(hListSelectedFilesForCertification, LB_GETTEXT, (WPARAM)i, (LPARAM)ListBoxString.data()) != LB_ERR) {//получаем текст элемента, если он был получен успешно
									ListBoxSelectedItems.push_back(ListBoxString.data());
									ListBoxItems.push_back(ListBoxString.data());
								}
								else MessageDebug(L"Не удалось получить текст выбранного элемента!", L"Ошибка при получении текста выбранного элемента!");//в случае неудачного получения текста элемента
							}
							else MessageDebug(L"Не удалось узнать длинну текста выбранного элемента!", L"Ошибка получения длинны выбранного элемента!");//в случае неудачного получения длинны строки
						}
						else if (ItemIsSelected == 0) {// если не выбран
							WCHARVECTOR ListBoxString;// здесь будет хранится полученная строка из ListBox
							LRESULT TextLen = SendMessageW(hListSelectedFilesForCertification, LB_GETTEXTLEN, (WPARAM)i, NULL);//получаем длинну строки
							if (TextLen != LB_ERR) {// если не ошибка
								ListBoxString.resize(TextLen + 1);//инициализируем массив ListBoxString, к TextLen прибавляем 1, т.к. длинна, которую мы получили в предыдущем шаге не включает терминирующий ноль
								if (SendMessageW(hListSelectedFilesForCertification, LB_GETTEXT, (WPARAM)i, (LPARAM)ListBoxString.data()) != LB_ERR) {//получаем текст элемента, если он был получен успешно
									ListBoxItems.push_back(ListBoxString.data());
								}
								else MessageDebug(L"Не удалось получить текст невыбранного элемента!", L"Ошибка при получении текста невыбранного элемента!");//в случае неудачного получения текста элемента
							}
							else MessageDebug(L"Не удалось узнать длинну текста невыбранного элемента!", L"Ошибка получения длинны невыбранного элемента!");//в случае неудачного получения длинны строки
						}
						else if (ItemIsSelected == LB_ERR)MessageDebug(L"Не удалось узнать выбран ли элемент", L"Ошибка при распознавании выбранного элемента!");//в случае ошибки проверки выбран ли элемент
					}
					OutputDebugStringW(L"Все элементы: \n");
					for (LRESULT i = 0; i < ListBoxItems.size(); i++) {
						OutputDebugStringW(ListBoxItems[i].data());
						OutputDebugStringW(L"\n");
					}
					OutputDebugStringW(L"Выбранные элементы: \n");
					for (LRESULT i = 0; i < ListBoxSelectedItems.size(); i++) {
						OutputDebugStringW(ListBoxSelectedItems[i].data());
						OutputDebugStringW(L"\n");
					}
					#endif
					///////////////////////////////////////////////////////////////////////////////////////////////
					//конец отладочного блока, дальше идёт обычной блок, выполняющий реализацию кнопки "Удалить"//
					//////////////////////////////////////////////////////////////////////////////////////////////
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
							EnableWindow(hDeleteFile, FALSE);
							EnableWindow(hModifyFile, FALSE);
							EnableWindow(hSign, FALSE);
							EnableWindow(hClear, FALSE);
						}
						if (SendMessageW(hListSelectedFilesForCertification, LB_SETHORIZONTALEXTENT, MaxStringWhidth, 0) == LB_ERR)MessageError(L"Не удалось установить прокручиваемую область!", L"Ошибка установки прокручиваемой области!", hDlg);//реинициализируем горизонтальный скролбар ListBox новым значением длинны прокрутки
					}
					else MessageError(L"Не удалось получить количество элементов", L"Ошибка получения количества элементов!", hDlg);//в случае ошибки получения кол-ва элементов
					//начало нового отладочного блока, данный блок будет проверять все ли выбранные элементы были удалены и не были ли удалены лишние(невыбранные) элементы
					#ifdef _DEBUG
					OutputDebugStringW(L"Этап после удаления: \n");
					//начало блока для получения текущих элементов в списке
					ItemsCount = SendMessageW(hListSelectedFilesForCertification, LB_GETCOUNT, NULL, NULL);// получение кол-ва элементов(файлов) в списке
					WSTRINGARRAY ListBoxElementsLastModify;
					for (LRESULT i = 0; i < ItemsCount; i++) {
						WCHARVECTOR ListBoxString;// здесь будет хранится полученная строка из ListBox
						LRESULT TextLen = SendMessageW(hListSelectedFilesForCertification, LB_GETTEXTLEN, (WPARAM)i, NULL);//получаем длинну строки
						if (TextLen != LB_ERR) {// если не ошибка
							ListBoxString.resize(TextLen + 1);//инициализируем массив ListBoxString, к TextLen прибавляем 1, т.к. длинна, которую мы получили в предыдущем шаге не включает терминирующий ноль
							if (SendMessageW(hListSelectedFilesForCertification, LB_GETTEXT, (WPARAM)i, (LPARAM)ListBoxString.data()) != LB_ERR) {//получаем текст элемента, если он был получен успешно
								ListBoxElementsLastModify.push_back(ListBoxString.data());
								
							}
							else MessageDebug(L"Не удалось получить текст элемента!", L"Ошибка при получении текста элемента!");//в случае неудачного получения текста элемента
						}
						else MessageDebug(L"Не удалось узнать длинну текста элемента!", L"Ошибка получения длинны элемента!");//в случае неудачного получения длинны строки
					}
					//конец блока получения текущих элементов в списке
					//начало блока проверки 
					auto ExistIdenticalElements = [](WSTRINGARRAY &a, WSTRINGARRAY &b)->WSTRINGARRAY {// данный функтор сравнивает два массива и формирует массив с элементами, существующими в обоих массивах
						WSTRINGARRAY result;
						for (LRESULT i = 0; i < a.size(); i++) {
							for (LRESULT j = 0; j < b.size(); j++) {
								if (a[i] == b[j])result.push_back(a[i].data());
							}
						}
						return result;
					};
					auto IncorrectlyDeletedItems = [](WSTRINGARRAY &A, WSTRINGARRAY &B, WSTRINGARRAY &C)->WSTRINGARRAY{
						//описание параметров:
						//A - исходный массив
						//B - массив с элементами, подлежащеми удалению
						//C - массив с дополнением A до B, требующий проверки, сформированный другим кодом 
						//данный функтор предназначен для проверки, праивльно ли пересечены A и B,
						// из массивов A и B он формирует массив, такой что в нём содержаться только те элементы из A, которых нет в B, далее он сравнивает сформированный на предыдущем шаге массив с массивом C, 
						//если эти массивы равны, то он возвращает пустой массив, если не равны, то он возвращает те элементы, которые должны быть в C(кроме тех, которые там уже есть)
						WSTRINGARRAY Result, ArrayWithCorrectlyDeletedElements;
						auto ElementIsExistInArray = [](const WSTRINGARRAY &Array, const WSTRING &element)->bool {
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
					WSTRING DebugOutputString = L"Значение списка после удаления: \n";
					for (size_t i = 0; i < ListBoxElementsLastModify.size(); i++)DebugOutputString+=(ListBoxElementsLastModify[i]+L"\n");
					OutputDebugStringW(DebugOutputString.c_str());
					//ListBoxElementsLastModify.push_back(ListBoxSelectedItems[0]);
					WSTRINGARRAY NotDeletedElements = ExistIdenticalElements(ListBoxElementsLastModify, ListBoxSelectedItems);
					if (NotDeletedElements.size() > 0) {
						DebugOutputString = L"В списке содержатся неудалённые элементы: \n";
						for (size_t i = 0; i < NotDeletedElements.size(); i++)DebugOutputString += (NotDeletedElements[i] + L"\n");
						OutputDebugStringW(DebugOutputString.c_str());
					}
					ListBoxElementsLastModify.pop_back();
					WSTRINGARRAY InvalidDeletedElements = IncorrectlyDeletedItems(ListBoxItems, ListBoxSelectedItems, ListBoxElementsLastModify);
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
			}
	}
	return (INT_PTR)FALSE;
}

UINT CalcBItemWidth(HWND hLB, LPCTSTR Text) {
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
				if (hr != S_OK)MessageError(L"Не удалось установить заголовок для диалогового окна открытия файлов!", L"Ошибка установки заголовка!", hWnd, hr);
				hr = pFileOpen->SetOkButtonLabel(OKButtonTitle);
				if (hr != S_OK)MessageError(L"Не удалось установить текст кнопки OK для диалогового окна открытия файлов!", L"Ошибка установки текста кнопки OK!", hWnd, hr);
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
										else MessageError(L"Не удалось получить имя открытого вами файла, файл не будет добавлен!", L"Ошибка получения имени открытого файла!", hWnd, hr);
										MyFile->Release();
									}
									else MessageError(L"Не удалось получить один из открытых вами файлов, файл не будет добавлен!", L"Ошибка получения открытого файла!", hWnd, hr);
								}
							}
							else MessageError(L"Не удалось получить количество открытых вами файлов!", L"Ошибка получения кол-ва открытых файлов!", hWnd, hr);
							pItem->Release();
						}
						else MessageError(L"Не удалось получить открытые вами файлы!", L"Ошибка получения открытых вами файлов!", hWnd, hr);
					}
					else if (hr != HRESULT_FROM_WIN32(ERROR_CANCELLED)) MessageError(L"Не удалось показать диалоговое окно открытия файлов, дальнейшее открытие файлов невозможно!", L"Ошибка показа диалогового окна!", hWnd, hr);
					pFileOpen->Release();
				}
				else MessageError(L"Не удалось установить необходимые опции для файлового диалога, дальнейшее открытие файлов невозможно!", L"Ощибка установки опций!", hWnd, hr);
			}
			else MessageError(L"Не удалось установить фильтры допустимых файлов, невозможно добавить файлы!", L"Ошибка установки фильтров для файлового диалога!", hWnd, hr);
		}
		else MessageError(L"Не удалось создать объект CLSID_FileOpenDialog, дальнейшее открытие файлов невозможно!", L"Ошибка создания объекта CLSID_FileOpenDialog!", hWnd, hr);
		CoUninitialize();
	}
	else MessageError(L"Не удалось инициализировать библиотеку COM!", L"Ошибка инициализиции библиотеки COM!", hWnd, hr);
	return FilesNames;
}
LoadCertificateResult LoadCertificate(HWND hWnd, const WSTRING *CertificateHref, bool LoadCertificateFromCertStore, LPCWSTR password) {
	LoadCertificateResult LCR;
	if (PP.CertificateInCertStore) {
		
	}
	else {
		HANDLE hCertFile = CreateFileW((*CertificateHref).c_str(), GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (hCertFile != INVALID_HANDLE_VALUE) {
			DWORD dwFileSize = GetFileSize(hCertFile, nullptr);
			if (dwFileSize != INVALID_FILE_SIZE) {
				HANDLE hCertMapping = CreateFileMappingW(hCertFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
				if (hCertMapping != nullptr) {
					CRYPT_DATA_BLOB CertificateBlob;
					CertificateBlob.cbData = dwFileSize;
					CertificateBlob.pbData = (BYTE*)MapViewOfFile(hCertMapping, FILE_MAP_READ, 0, 0, dwFileSize);
					if (CertificateBlob.pbData != nullptr) {
						LCR.hCertStore = PFXImportCertStore(&CertificateBlob, password, PKCS12_NO_PERSIST_KEY);
						if (LCR.hCertStore != NULL) {
							LCR.pCertContext = CertFindCertificateInStore(LCR.hCertStore, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, 0, CERT_FIND_ANY, NULL, NULL);
							if (LCR.pCertContext != nullptr)LCR.CertificateIsLoaded = true;
							else {
								WSTRING ErrorText = L"Не удалось получить загрузить сертификат \"" + (*CertificateHref) + L"\"!";
								LCR.ErrorInfo = ErrorString(ErrorText.c_str());
								LCR.ErrorInfo.ErrorCaption = L"Ошибка загрузки сертификата!";
							}
							//CertCloseStore(hCertStore, CERT_CLOSE_STORE_FORCE_FLAG);
						}
						UnmapViewOfFile(CertificateBlob.pbData);
					}
					else {
						WSTRING ErrorText = L"Не удалось получить указатель на участок в памяти с отображением файла \"" + (*CertificateHref) + L"\"!";
						LCR.ErrorInfo = ErrorString(ErrorText.c_str());
						LCR.ErrorInfo.ErrorCaption = L"Ошибка получения указателя!";
					}
					CloseHandle(hCertMapping);
				}
				else {
					WSTRING ErrorText = L"Не отобразить файл сертификата \"" + (*CertificateHref) + L"\" в память";
					LCR.ErrorInfo = ErrorString(ErrorText.c_str());
					LCR.ErrorInfo.ErrorCaption = L"Ошибка отображения файла в память!";
				}
			}
			else {
				WSTRING ErrorText = L"Не получить размер файла сертификата \"" + (*CertificateHref) + L"\"";
				LCR.ErrorInfo = ErrorString(ErrorText.c_str());
				LCR.ErrorInfo.ErrorCaption = L"Ошибка получения размера файла сертификата";
			}
			CloseHandle(hCertFile);
		}
		else {
			WSTRING ErrorText = L"Не удалось открыть файл сертификата \"" + (*CertificateHref) + L"\"";
			LCR.ErrorInfo = ErrorString(ErrorText.c_str());
			LCR.ErrorInfo.ErrorCaption = L"Ошибка открытия файла!";
		}
		//if (pCertContext)CertFreeCertificateContext(pCertContext);
		
		
	}
	return LCR;
}
/*
	Данная функция подписывает файл сертификатом, она принимает
		SF - указатель на структуру SIGN_FILE_RESULT::SIGNING_FILE, которая описывает подписываемый файл(подробности смотрите в объявлении этой структуры
		SI - указатель на инициализационную структуру SIGN_FILE_RESULT::PCSIGN_FILE_INIT, подробности об этой структуре смотрите в комментариях к её определению
	Считается, что данная функция успешно завершила инициализацию, если:
		1)SI->pfSignerSign не равен nullptr и указывает на действительный адресс соответсвующей функции
		2)SF->SignCertificate не равен nullptr и указывает на действительный адресс сертификата
*/
SIGN_FILE_RESULT SignFile(SIGN_FILE_RESULT::PCSIGNING_FILE SF, SignerSignType pfSignerSign) {
	SIGN_FILE_RESULT result;
	SIGNER_FILE_INFO signerFileInfo;
	SIGNER_SUBJECT_INFO signerSubjectInfo;
	SIGNER_CERT_STORE_INFO signerCertStoreInfo;
	SIGNER_CERT signerCert;
	SIGNER_SIGNATURE_INFO signerSignatureInfo;
	if (pfSignerSign) {
		DWORD dwIndex = 0;
		signerFileInfo.cbSize = sizeof(SIGNER_FILE_INFO);
		signerFileInfo.pwszFileName = SF->SigningFileName;
		signerFileInfo.hFile = NULL;
		// Prepare SIGNER_SUBJECT_INFO struct
		signerSubjectInfo.cbSize = sizeof(SIGNER_SUBJECT_INFO);
		signerSubjectInfo.pdwIndex = &dwIndex;
		signerSubjectInfo.dwSubjectChoice = 1; // SIGNER_SUBJECT_FILE
		signerSubjectInfo.pSignerFileInfo = &signerFileInfo;
		// Prepare SIGNER_CERT_STORE_INFO struct
		signerCertStoreInfo.cbSize = sizeof(SIGNER_CERT_STORE_INFO);
		signerCertStoreInfo.pSigningCert = SF->SignCertificate;
		signerCertStoreInfo.dwCertPolicy = 2; // SIGNER_CERT_POLICY_CHAIN
		signerCertStoreInfo.hCertStore = NULL;
		// Prepare SIGNER_CERT struct
		signerCert.cbSize = sizeof(SIGNER_CERT);
		signerCert.dwCertChoice = 2; // SIGNER_CERT_STORE
		signerCert.pCertStoreInfo = &signerCertStoreInfo;
		signerCert.hwnd = NULL;
		// Prepare SIGNER_SIGNATURE_INFO struct
		signerSignatureInfo.cbSize = sizeof(SIGNER_SIGNATURE_INFO);
		signerSignatureInfo.algidHash = CALG_SHA1;
		signerSignatureInfo.psAuthenticated = NULL;
		signerSignatureInfo.psUnauthenticated = NULL;
		signerSignatureInfo.dwAttrChoice = NULL; // SIGNER_NO_ATTR
		signerSignatureInfo.pAttrAuthcode = NULL;
		HRESULT hSignerSignResult = pfSignerSign(&signerSubjectInfo, &signerCert, &signerSignatureInfo, NULL, NULL, NULL, NULL);
		if (SUCCEEDED(hSignerSignResult)) {
			//SI->pfSignerFreeSignerContext(pSignerContext);
			result.FileIsSigned = true;
		}
		else {
			//в случае если неудалось подписать файл
			WSTRING ErrorText = L"Не удалось подписать файл \"";
			ErrorText += SF->SigningFileName;
			ErrorText += L"\" цифровой подписью!";
			result.ErrorInfo.ErrorString = ErrorString(ErrorText.c_str(), hSignerSignResult);
			result.ErrorInfo.ErrorCaption = L"Ошибка подписи файла";
			result.ErrorInfo.ErrorCode = hSignerSignResult;
		}
	}
	else {
		result.ErrorInfo = ErrorString(L"Неверный адресс функции SignerSignEx, он не может равняться nullptr!");
		result.ErrorInfo.ErrorCaption = L"Неверный адресс функции!";
		result.InitializationFailed = true;
	}
	return result;
}
bool ThisStringIsProgrammArgument(WSTRING arg) {
	for (size_t i = 0; i < ProgrammSettings::CommandLineValidArguments.size(); i++)if (arg == ProgrammSettings::CommandLineValidArguments[i])return true;
	return false;
}