// SignProjectTool.cpp : Определяет точку входа для приложения.
//

#include "stdafx.h"
#include "SignProjectTool.h"
#define MAX_LOADSTRING 100
//Макрос, определяющий корневую ветку в реестре для данной программы, он будет использоваться относительно HKEY_CURRENT_USER
#define ROOT_REGISTRY_KEY_SIGNPROJECT_TOOL L"Software\\Blbulyan Software\\SignProjectTool"
using namespace WWS;
//данные функции используются в структурах, которые описаны ниже, поэтому их прототипы на самом верху
bool CheckBit(unsigned word, unsigned bit);
bool FileReadebleExists(LPCTSTR fname);//функция проверяет существует ли файл и доступен ли он для записи
//структура с основными параметрами программы
struct ProgrammSettings {
	void SaveProgrammSettingsInRegistry(unsigned sfSave);//данная функция преназначена для сохранения параметров программы в реестр Windows
	void LoadProgrammSettingsOfRegistry(unsigned sfLoad);//данная функция предназначена для загрузки параметров программы из реестра Windows
	enum SETTINGS_TO_SAVE{SIGN_CERTIFICATE_FILE_NAME = 0b00000001, ALG_HASH = 0b00000010};//перечисление, оперделяющее флаги, комбинация которых передаётся вышеописанным функциям, они позволяют задать какие параметры следует (загружать из реестра)/(сохранять в реестре)
	//флаги комбинировать с помощью операции побитового ИЛИ, пример вызова LoadProgrammSettingsOfRegistry с заданными параметрами:
	//LoadProgrammSettingsOfRegistry(SIGN_CERTIFICATE_FILE_NAME | ALG_HASH) - функция загрузит два параметра из реестра, путь к сертификату и алгоритм хеша
	//LoadProgrammSettingsOfRegistry(SIGN_CERTIFICATE_FILE_NAME) - функция загрузит только полный путь к сертификату
	//LoadProgrammSettingsOfRegistry(ALG_HASH) - функция загрузит только алгоритм хеширования
	//соотвственно флаг SIGN_CERTIFICATE_FILE_NAME - предназначен для загрузки полного пути к сертификату для подписи из реестра Windows,
	//а флаг ALG_HASH - предназначен для загрузки алгоритма хеширования из реестра Windows
	enum COMMAND_AFTER_SIGNING_FILES {
		CASF_DELETE_SUCCESSFULLY_SIGNED_FILES_FROM_LIST = 0b0000000000000001,
		CASF_DELETE_NOT_SUCCESSFULLY_SIGNED_FILES_FROM_LIST = 0b0000000000000010,
		CASF_DELETE_ALL_FILES_FROM_LIST = 0b0000000000000011
	};
	/*
		Команды, которые могут быть выполнены над списком:
			0bxxxxxx00 - ничего не удалять из списка
			0bxxxxxxx1 - удалить успешно подписанные файлы из списка
			0b00000010 - удалить недаучноподписанные файлы из списка
			0bxxxxxx11 - удалить все файлы из списка

	*/
	WSTRINGARRAY FilesForCertification;//список файлов для подписи
	WSTRING CertificateFile;// имя файла сертификата для подписи им
	WSTRINGARRAY HttpTimeStampServers;//список TimeStamp серверов
	static const WSTRINGARRAY CommandLineValidArguments;//массив с допустимыми аргументами командной строки
	bool CertificateInCertStore = false;//хранится ли сертификат, которым будут подписываться файлы в хранилище сертификатов (задаётся при начальной настройке программы
	ALG_ID HashAlgorithmId = CALG_SHA_256;//алгоритм хеширования при подписи поумолчанию
	bool LoadSettingsFromRegistry = true;//загружать настройки из реестра Windows
	bool SaveSettingsInRegistry = true;//сохранять настройки в реестр Windows
	WORD CommandToExecuteAfterSigningFiles = 0;//команда для исполнения после подписи файлов
	

} PP;
//структура описывающая элементы рантайма, состояние программы в рантайме, данные в ней могут менятся в зависимости от запуска к запуску
struct ProgrammStatement {
	HINSTANCE hInst = NULL;//экземпляр приложения
	WNDPROC DefaultConsoleWindowProc = NULL;//процедура консоли по умолчанию(возможно не используется)
	HANDLE hSignFilesThread = nullptr;//дескриптор этого потока
	BOOL ConsoleIsAlloced = FALSE;//выделена ли консоль
	HWND hRootWnd = NULL;//дескриптор главного окна
} PS;
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
//структура с параметрами для потока, выполняющего подпись файлов
struct PARAMETERS_FOR_THREAD_SIGN_FILES {
	enum THREAD_MESSAGES {THM_CANCEL, THM_CLOSE};//перечисление с основными сообщениями для потока
	/*
		THM_CANCEL - посылается в случае отмены операции подписи файлов
		THM_CLOSE - посылается в случае закрытия приложения
	*/
	HWND hDlg = NULL;//дескриптор диалога, относительно которого будут выводится сообщения
	HWND hStatusBar = NULL;//дескриптор окна статус бара, в который выводится информация о ходе подписи
	HANDLE hOutputConsole = nullptr;//дескриптор вывода консоли
	HANDLE hErrorConsole = nullptr;//дескриптор вывода ошибок консоли
	HMODULE hMssign32 = NULL;//дескриптор библиотеки Mssign32.dll
	LRESULT ItemsCount = 0;//количество элементов в списке hListSelectedFilesForCertification
	PCCERT_CONTEXT SignCertificate = nullptr;//сертификат для подписи
	HCERTSTORE hCertStore = nullptr;//хранилище сертификатов, в котором хранится сертификат SignCertificate
	SignerSignType pfSignerSign = nullptr;// указатель на функциюю SignerSign
	byte CommandToExecuteAfterSigningFiles = 0b00000000;//команда для исполнения после подписи файлов(детали описаны в комментарии в структуре с основными параметрами программы)
	typedef PARAMETERS_FOR_THREAD_SIGN_FILES* PPARAMETERS_FOR_THREAD_SIGN_FILES;
};
//данную структуру "возвращает" диалог стартовой конфигурации
struct START_CONFIGURATION_RESULT {
	WSTRING CertificateFileName;
	ALG_ID HashAlgoritmId = NULL;
	WSTRINGARRAY TimeStampServers;
	typedef START_CONFIGURATION_RESULT* PSTART_CONFIRURATION_RESULT;
};

// Отправить объявления функций, включенных в этот модуль кода:
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK StartConfigurationDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);// процедура обработки сообщений диалога IDD_START_CONFIGURATION
INT_PTR CALLBACK AddFilesForCertificationDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);// процедура обработки сообщений диалога IDD_ADD_FILES_FOR_CERTIFICATION
INT_PTR CALLBACK SettingsTimeStampsServersDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);//процедура обработки сообщений диалога IDD_SETTINGS_TIMESTAMP_SERVERS
//INT_PTR CALLBACK MainConsoleWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK SettingsForActionsPerformedAfterSigningDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
UINT CalcBItemWidth(HWND hWnd, LPCTSTR Text);//функция считает размер строки в пикселях, вызывается перед добавлением строки в ListBox, затем данный размер использует в качестве значения горизонтальной прокрутки
WSTRINGARRAY OpenFiles(HWND hWnd, COMDLG_FILTERSPEC *cmf, UINT cmfSize, LPCWSTR TitleFileDialog, LPCWSTR OKButtonTitle, bool Multiselect);//функция предназначена для показа диалогового окна выбора файлов/файла, возвращает массив строк с полными путями к файлам/возвраает массив строк, содержащий один путь к выбранному файлу, соответсвенно
SIGN_FILE_RESULT SignFile(SIGN_FILE_RESULT::PCSIGNING_FILE SF, SignerSignType pfSignerSign);//функция предназначена для подписи файла
LoadCertificateResult LoadCertificate(HWND hWnd, const WSTRING *CertificateHref, bool LoadCertificateFromCertStore, LPCWSTR password);//функция предназначена для загрузки сертификата из файла/хранилища(загрузка из хранилища не реализована)
bool ThisStringIsProgrammArgument(WSTRING arg);//проверка является ли строка аргументом программы
INT StringExistInListBox(HWND hListBox, WSTRING str);//существует ли строка в ListBox
DWORD WINAPI SignFilesThreadProc(_In_ LPVOID lpParameter);//поточная процедура, отвечающая за подпись файлов
void EnableWindowClose(HWND hwnd);//функция включает кнопку закрытия на окне
void DisableWindowClose(HWND hwnd);//функция отключает кнопку закрытия на окне
BOOL CALLBACK FindListBoxChildWindowProc(HWND hWnd, LPARAM lParam);//процедура обратного вызова для функции EnumChildWindows, её цель - найти ListBox в дочерних окнах и вернуть на него первый попавшейся дескриптор, в качестве lParam ожидает указаьтель на переменную типа HWND для записи в неё результата, используется с EnumChildWindows
BOOL WINAPI ConsoleHandler(_In_ DWORD dwCtrlType);
int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow){
    UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);
	unsigned sfLoadParametersFromRegistry = ProgrammSettings::SETTINGS_TO_SAVE::ALG_HASH | ProgrammSettings::SETTINGS_TO_SAVE::SIGN_CERTIFICATE_FILE_NAME;//данная переменная хранит в себе то,
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
						sfLoadParametersFromRegistry ^= ProgrammSettings::SETTINGS_TO_SAVE::SIGN_CERTIFICATE_FILE_NAME;
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
	//if (PP.CertificateFile != L"")PP.SaveProgrammSettingsInRegistry(ProgrammSettings::SETTINGS_TO_SAVE::SIGN_CERTIFICATE_FILE_NAME | ProgrammSettings::SETTINGS_TO_SAVE::ALG_HASH);
	if(PP.SaveSettingsInRegistry)PP.SaveProgrammSettingsInRegistry(ProgrammSettings::SETTINGS_TO_SAVE::ALG_HASH);
	return msg.wParam;
}

// Обработчик сообщений для окна "О программе".
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
		case WM_INITDIALOG:{
			HICON hDialogIcon = LoadIconW(PS.hInst, MAKEINTRESOURCEW(IDI_SIGNPROJECTTOOL));
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
	HWND hOpenFromCertStore = GetDlgItem(hDlg, IDC_BUTTON_SELECT_CERTIFICATE_FROM_STORE), hOpenFile = GetDlgItem(hDlg, IDC_OPEN_FILE), hCertFile = GetDlgItem(hDlg, IDC_CERT_FILE), hCertStore = GetDlgItem(hDlg, IDC_CERT_STORE), hCertList = GetDlgItem(hDlg, IDC_COMBOX_LISTCERT_FILES);
	//static WSTRING CertificateFileName;
	static UINT MaxHScrollWidth = NULL;
	static START_CONFIGURATION_RESULT LSCR;
	static START_CONFIGURATION_RESULT::PSTART_CONFIRURATION_RESULT PSCR = NULL;
	switch(message){
		case WM_INITDIALOG:{
			PSCR = (START_CONFIGURATION_RESULT::PSTART_CONFIRURATION_RESULT)lParam;
			//CertificateFileName = PP.CertificateFile;
			HICON hDialogIcon = LoadIconW(PS.hInst, MAKEINTRESOURCEW(IDI_SIGNPROJECTTOOL));
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
				case IDC_OPEN_CERT_FILE:{
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
							 if (AddStringResult == CB_ERR || AddStringResult == CB_ERRSPACE)MessageError(L"Ошибка добавления строки с именем файла сертификат с соответсвующий список!", L"Ошибка при добавлении стрроки в combobox", hDlg);
							 else {
								 HWND hListBoxInComboBox = NULL;
								 SendMessageW(hCertList, CB_SELECTSTRING, -1, (LPARAM)CertificateFile[0].c_str()); 
								 /*Данная часть кода предназначена для включения горизонтальной прокрутки в ComboBox (НЕ РАБОТАЕТ)*/
								 EnumChildWindows(hCertList, FindListBoxChildWindowProc, (LPARAM)& hListBoxInComboBox);
								 if (hListBoxInComboBox != NULL) {
									 UINT LocalMaxHScrollWidth = CalcBItemWidth(hListBoxInComboBox, CertificateFile[0].c_str());
									 if (LocalMaxHScrollWidth > MaxHScrollWidth) {
										 MaxHScrollWidth = LocalMaxHScrollWidth;
										 SendMessageW(hCertList, CB_SETHORIZONTALEXTENT, MaxHScrollWidth, 0);
									 }
								 }
								
							 }
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
									
									//EnableWindow(hProgressForSigningFiles, TRUE);
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
												HWND hConsoleWindow = GetConsoleWindow();
												if (hConsoleWindow != NULL)DisableWindowClose(hConsoleWindow);
												else MessageError(L"Не удалось получить дескриптор окна консоли для отключения кнопки закрытия!", L"Ошибка получения дескриптора!", hDlg);
											}
											//DisableWindowClose(hDlg);
											SF.SignCertificate = LCR.pCertContext;
										
											TSFINIT.hDlg = hDlg;
											TSFINIT.hStatusBar = hStatusBar;
											TSFINIT.pfSignerSign = pfSignerSign;
											TSFINIT.hCertStore = LCR.hCertStore;
											TSFINIT.SignCertificate = LCR.pCertContext;
											
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
						INT_PTR ResultDialog = DialogBoxParamW(PS.hInst, MAKEINTRESOURCEW(IDD_SETTINGS_FOR_ACTIONS_PERFORMED_AFTER_SIGNING), hDlg, SettingsForActionsPerformedAfterSigningDlgProc, PP.CommandToExecuteAfterSigningFiles);
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
//процедура обработки сообщений 
INT_PTR CALLBACK SettingsTimeStampsServersDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam){
	HWND hClearInput = GetDlgItem(hDlg, IDC_CLEAR_INPUT), hAddTimeStampServer = GetDlgItem(hDlg, IDC_ADD_TIMESTAMP_SERVER), hDeleteTimeStampServer = GetDlgItem(hDlg, IDC_DELETE_TIMESTAMP_SERVER), hClearTimeStampServers = GetDlgItem(hDlg, IDC_CLEAR_TIMESTAMP_SERVERS), hListTimeStampServers = GetDlgItem(hDlg, IDC_LIST_TIMESTAMP_SERVERS), hEditTimeStampServer = GetDlgItem(hDlg, IDC_EDIT_TIMESTAMP_SERVER), hItemsCount = GetDlgItem(hDlg, IDC_ITEMS_COUNT);
	static UINT MaxStringWhidth = 0;//переменная характеризует максимально возможное значение, на которое можно прокрутить горизонтальный скролбар ListBox
	static WSTRINGARRAY *TimeStampList = nullptr;
	switch (message){
		case WM_INITDIALOG: {
			HICON hDialogIcon = LoadIconW(PS.hInst, MAKEINTRESOURCEW(IDI_SIGNPROJECTTOOL));
			if (hDialogIcon == NULL)MessageError(L"Не удалось загрузить иконку для текущего диалога!", L"Ошибка загрузки иконки!", hDlg);
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
					else MessageError(L"Не удалось получить количество выбранных элементов", L"Ошибка получения количества элементов!", hDlg);//в случае ошибки получения кол-ва элементов
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
								else MessageError(L"Не удалось получить текст невыбранного элемента!", L"Ошибка при получении текста невыбранного элемента!", hDlg);//в случае неудачного получения текста элемента
							}
							else MessageError(L"Не удалось узнать длинну текста невыбранного элемента!", L"Ошибка получения длинны невыбранного элемента!", hDlg);//в случае неудачного получения длинны строки
						}
					}
					else MessageError(L"Не удалось получить количество элементов", L"Ошибка получения количества элементов!", hDlg);//в случае ошибки получения кол-ва элементов
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
								else MessageError(L"Не удалось удалить выбранный элемент из списка файлов!", L"Ошибка удаления выбранного элемента!", hDlg);// в случае ошибки
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
									else MessageError(L"Не удалось получить текст невыбранного элемента!", L"Ошибка при получении текста невыбранного элемента!", hDlg);//в случае неудачного получения текста элемента
								}
								else MessageError(L"Не удалось узнать длинну текста невыбранного элемента!", L"Ошибка получения длинны невыбранного элемента!", hDlg);//в случае неудачного получения длинны строки
							}
							else if (ItemIsSelected == LB_ERR)MessageError(L"Не удалось узнать выбран ли элемент", L"Ошибка при распознавании выбранного элемента!", hDlg);//в случае ошибки проверки выбран ли элемент
						}
						if (ItemsCount == 0) {// если список стал пустым
							//отключение всех органов управления, для которых требуется наличие хотя бы одного файла в списке
							EnableWindow(hDeleteTimeStampServer, FALSE);
							EnableWindow(hClearTimeStampServers, FALSE);
						}
						WSTRING ItemsCountText = L"Список TimeStamp серверов(" + std::to_wstring(ItemsCount) + L"):";
						SetWindowTextW(hItemsCount, ItemsCountText.c_str());
						if (SendMessageW(hListTimeStampServers, LB_SETHORIZONTALEXTENT, MaxStringWhidth, 0) == LB_ERR)MessageError(L"Не удалось установить прокручиваемую область!", L"Ошибка установки прокручиваемой области!", hDlg);//реинициализируем горизонтальный скролбар ListBox новым значением длинны прокрутки
					}
					else MessageError(L"Не удалось получить количество элементов", L"Ошибка получения количества элементов!", hDlg);//в случае ошибки получения кол-ва элементов
					break;
				}
				case IDC_CLEAR_INPUT:
					SetWindowTextW(hEditTimeStampServer, L"");
					break;
				case IDC_CLEAR_TIMESTAMP_SERVERS:
					if (SendMessageW(hListTimeStampServers, LB_RESETCONTENT, NULL, NULL) == LB_ERR)MessageError(L"Не удалось очистить список!", L"Ошибка очистки списка!", hDlg);
					MaxStringWhidth = 0;//обнуление длинны прокрутки горизонтального скролбара ListBox
					// "выключение" горизонтального скролбара
					if (SendMessageW(hListTimeStampServers, LB_SETHORIZONTALEXTENT, MaxStringWhidth, 0) == LB_ERR)MessageError(L"Не удалось установить прокручиваемую область!", L"Ошибка установки прокручиваемой области!", hDlg);
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
									if (SendMessageW(hListTimeStampServers, LB_SETHORIZONTALEXTENT, MaxStringWhidth, 0) == LB_ERR)MessageError(L"Не удалось установить прокручиваемую область!", L"Ошибка установки прокручиваемой области!", hDlg);//реинициализируем горизонтальный скролбар ListBox новым значением длинны прокрутки
									EnableWindow(hClearTimeStampServers, TRUE);
									SetWindowTextW(hEditTimeStampServer, L"");
									LRESULT ItemsCount = SendMessageW(hListTimeStampServers, LB_GETCOUNT, NULL, NULL);// получение кол-ва элементов(файлов) в списке
									if (ItemsCount != LB_ERR) {
										WSTRING ItemsCountText = L"Список TimeStamp серверов(" + std::to_wstring(ItemsCount) + L"):";
										SetWindowTextW(hItemsCount, ItemsCountText.c_str());
									}
								}
								else MessageError(L"Не удалось добавить TimeStamp сервер в список!", L"Ошибка добавления элемента в список!", hDlg);
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
INT_PTR CALLBACK SettingsForActionsPerformedAfterSigningDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam){
	static WORD result = 0;
	switch (message) {
		case WM_INITDIALOG: {
			HICON hDialogIcon = LoadIconW(PS.hInst, MAKEINTRESOURCEW(IDI_SIGNPROJECTTOOL));
			if (hDialogIcon == NULL)MessageError(L"Не удалось загрузить иконку для текущего диалога!", L"Ошибка загрузки иконки!", hDlg);
			else {
				SendMessageW(hDlg, WM_SETICON, ICON_BIG, (LPARAM)hDialogIcon);
				SendMessageW(hDlg, WM_SETICON, ICON_SMALL, (LPARAM)hDialogIcon);
			}
			result = lParam;
			//начало инициализации радиокнопок
			if (result & ProgrammSettings::COMMAND_AFTER_SIGNING_FILES::CASF_DELETE_ALL_FILES_FROM_LIST == ProgrammSettings::COMMAND_AFTER_SIGNING_FILES::CASF_DELETE_ALL_FILES_FROM_LIST)CheckDlgButton(hDlg, IDC_RADIO_DELETE_ALL_FILES_FROM_LIST, BST_CHECKED);
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
/*INT_PTR CALLBACK MainConsoleWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch(message){
		case WM_CLOSE:
			FreeConsole();
			return 0;
	}
	return PP.DefaultConsoleWindowProc(hWnd, message, wParam, lParam);
}*/
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
	if (LoadCertificateFromCertStore) {
		
	}
	else {
		HANDLE hCertFile = CreateFileW((*CertificateHref).c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
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
								WSTRING ErrorText = L"Не удалось загрузить сертификат \"" + (*CertificateHref) + L"\"!";
								LCR.ErrorInfo = ErrorString(ErrorText.c_str());
								LCR.ErrorInfo.ErrorCaption = L"Ошибка загрузки сертификата!";
							}
							//CertCloseStore(hCertStore, CERT_CLOSE_STORE_FORCE_FLAG);
						}
						else {
							WSTRING ErrorText = L"Не удалось загрузить сертификат \"" + (*CertificateHref) + L"\"!";
							LCR.ErrorInfo = ErrorString(ErrorText.c_str());
							LCR.ErrorInfo.ErrorCaption = L"Ошибка загрузки сертификата!";
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
		pfSignerSign - указатель на функцию SignerSign
	Считается, что данная функция успешно завершила инициализацию, если:
		1)pfSignerSign не равен nullptr и указывает на действительный адресс соответсвующей функции
		2)SF->SignCertificate не равен nullptr и указывает на действительный адресс сертификата
*/
SIGN_FILE_RESULT SignFile(SIGN_FILE_RESULT::PCSIGNING_FILE SF, SignerSignType pfSignerSign) {
	SIGN_FILE_RESULT result;
	SIGNER_FILE_INFO signerFileInfo;
	SIGNER_SUBJECT_INFO signerSubjectInfo;
	SIGNER_CERT_STORE_INFO signerCertStoreInfo;
	SIGNER_CERT signerCert;
	SIGNER_SIGNATURE_INFO signerSignatureInfo;
	//SIGNER_PROVIDER_INFO signerProviderInfo;
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
		signerSignatureInfo.algidHash = SF->HashAlgorithmId;
		signerSignatureInfo.psAuthenticated = NULL;
		signerSignatureInfo.psUnauthenticated = NULL;
		signerSignatureInfo.dwAttrChoice = NULL; // SIGNER_NO_ATTR
		signerSignatureInfo.pAttrAuthcode = NULL;
		/*signerProviderInfo.cbSize = sizeof(SIGNER_PROVIDER_INFO);
		signerProviderInfo.dwProviderType = PROV_RSA_AES;
		signerProviderInfo.pwszProviderName = MS_ENH_RSA_AES_PROV;
		signerProviderInfo.dwPvkChoice = 0x1;
		signerProviderInfo.pwszPvkFileName = (LPWSTR)L"C:\\Users\\blbul\\OneDrive\\Документы\\Сертификаты\\CyberForum.pvk";*/
		HRESULT hSignerSignResult = pfSignerSign(&signerSubjectInfo, &signerCert, &signerSignatureInfo, NULL, NULL, NULL, NULL);
		if (SUCCEEDED(hSignerSignResult))result.FileIsSigned = true;
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

INT StringExistInListBox(HWND hListBox, WSTRING str)
{
	if (hListBox != NULL) {
		if (SendMessageW(hListBox, LB_FINDSTRINGEXACT, -1, (LPARAM)str.c_str()) == LB_ERR)return FALSE;
		else return TRUE;
	}
	return -1;
}
//функция отключает кнопку закрытия на окне
void DisableWindowClose(HWND hwnd){
	// Отключить кнопку закрыть.
	SetClassLongPtr(hwnd, GCL_STYLE, GetClassLongPtr(hwnd, GCL_STYLE) | CS_NOCLOSE);
	// Отключить системное меню пункт.
	EnableMenuItem(GetSystemMenu(hwnd, FALSE), SC_CLOSE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
}
BOOL CALLBACK FindListBoxChildWindowProc(HWND hWnd, LPARAM lParam)
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
}
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
//функция включает кнопку закрытия на окне
void EnableWindowClose(HWND hwnd){
	// Включить кнопку закрыть.
	SetClassLongPtr(hwnd, GCL_STYLE, GetClassLongPtr(hwnd, GCL_STYLE) & ~CS_NOCLOSE);
	// Включить пункт системного меню.
	EnableMenuItem(GetSystemMenu(hwnd, FALSE), SC_CLOSE, MF_BYCOMMAND | MF_ENABLED);
}
//поточная процедура, отвечающая за подпись файлов
DWORD WINAPI SignFilesThreadProc(_In_ LPVOID lpParameter) {
	PARAMETERS_FOR_THREAD_SIGN_FILES::PPARAMETERS_FOR_THREAD_SIGN_FILES PFSF = (PARAMETERS_FOR_THREAD_SIGN_FILES::PPARAMETERS_FOR_THREAD_SIGN_FILES)lpParameter;
	HWND hListSelectedFilesForCertification = GetDlgItem(PFSF->hDlg, IDC_LIST_SELECTED_FILES_FOR_CERTIFICTION), hDeleteFile = GetDlgItem(PFSF->hDlg, IDC_DELETE_FILE), hModifyFile = GetDlgItem(PFSF->hDlg, IDC_MODIFY_FILE), hSign = GetDlgItem(PFSF->hDlg, IDC_SIGN), hClear = GetDlgItem(PFSF->hDlg, IDC_CLEAR), hQuitCancel = GetDlgItem(PFSF->hDlg, IDCANCEL), hProgressForSigningFiles = GetDlgItem(PFSF->hDlg, IDC_PROGRESS_SIGNING_FILES), hAddFiles = GetDlgItem(PFSF->hDlg, IDC_ADD_FILE), hPauseSigning = GetDlgItem(PFSF->hDlg, IDC_PAUSE_SIGNING);
	SIGN_FILE_RESULT::SIGNING_FILE SF;
	BOOL ButtonModifyFileLastStatus, ButtonDeleteFileLastStatus;
	BOOL AnErrorHasOccurred = FALSE;
	SF.SignCertificate = PFSF->SignCertificate;
	MSG msg;
	//подготовка индикационных элементов
	SetWindowTextW(hQuitCancel, L"Отмена");
	SendMessageW(hProgressForSigningFiles, PBM_SETRANGE32, 0, PFSF->ItemsCount);
	SendMessageW(hProgressForSigningFiles, PBM_SETPOS, 0, 0);
	SendMessageW(hProgressForSigningFiles, PBM_SETSTEP, (WPARAM)1, NULL);
	SendMessageW(PFSF->hStatusBar, SB_SETTEXTW, MAKEWORD(0, SBT_POPOUT), (LPARAM)L"Идёт подпись файлов");
	//отключение необходимых элементов управления на время подписи
	EnableWindow(hSign, FALSE);
	EnableWindow(hClear, FALSE);
	ButtonModifyFileLastStatus = EnableWindow(hModifyFile, FALSE);
	ButtonDeleteFileLastStatus = EnableWindow(hDeleteFile, FALSE);
	EnableWindow(hAddFiles, FALSE);
	//Включение необходимых элементов на время подписи
	EnableWindow(hPauseSigning, TRUE);
	for (LRESULT i = 0; i < PFSF->ItemsCount; i++) {//цикл подписи файлов
		if (PeekMessageW(&msg, NULL, PARAMETERS_FOR_THREAD_SIGN_FILES::THREAD_MESSAGES::THM_CANCEL, PARAMETERS_FOR_THREAD_SIGN_FILES::THREAD_MESSAGES::THM_CLOSE, PM_REMOVE)) {
			switch (msg.message){
				case PARAMETERS_FOR_THREAD_SIGN_FILES::THREAD_MESSAGES::THM_CLOSE://данное сообщение передаётся при нажатии на кнопку закрытия
					if (PFSF->SignCertificate != NULL)CertFreeCertificateContext(PFSF->SignCertificate);
					if (PFSF->hCertStore != NULL)CertCloseStore(PFSF->hCertStore, CERT_CLOSE_STORE_FORCE_FLAG);
					if(PFSF->hStatusBar != NULL)SendMessageW(PFSF->hStatusBar, SB_SETTEXTW, MAKEWORD(0, SBT_POPOUT), (LPARAM)L"Завершение приложения");
					if (PFSF->hMssign32 != NULL)FreeLibrary(PFSF->hMssign32);
					return 0;
				case PARAMETERS_FOR_THREAD_SIGN_FILES::THREAD_MESSAGES::THM_CANCEL:{//данное сообщение передаётся при нажатии на кнопку отмены
					SetWindowTextW(hQuitCancel, L"Выход");
					HWND hConsoleWindow = (PS.ConsoleIsAlloced == TRUE ? GetConsoleWindow() : NULL);
					if (hConsoleWindow != NULL)EnableWindowClose(hConsoleWindow);
					EnableWindowClose(PFSF->hDlg);
					if (PFSF->SignCertificate != NULL)CertFreeCertificateContext(PFSF->SignCertificate);
					if (PFSF->hCertStore != NULL)CertCloseStore(PFSF->hCertStore, CERT_CLOSE_STORE_FORCE_FLAG);
					if (PFSF->hStatusBar != NULL)SendMessageW(PFSF->hStatusBar, SB_SETTEXTW, MAKEWORD(0, SBT_POPOUT), (LPARAM)L"Подпись отменена");
					if (PS.ConsoleIsAlloced == TRUE)FreeConsole();
					SendMessageW(hProgressForSigningFiles, PBM_SETPOS, 0, 0);
					EnableWindow(hSign, TRUE);
					EnableWindow(hClear, TRUE);
					//возвращаем предыдущее состояние кнопок удалить и изменить
					if (ButtonDeleteFileLastStatus == NULL)EnableWindow(hDeleteFile, TRUE);
					if (ButtonModifyFileLastStatus == NULL)EnableWindow(hModifyFile, TRUE);
					//конец возвращения предыдущего состояния
					EnableWindow(hAddFiles, TRUE);
					EnableWindow(hPauseSigning, FALSE);
					if(PFSF->hMssign32 != NULL)FreeLibrary(PFSF->hMssign32);
					return ERROR_CANCELLED;
				}
			}
		}
		WCHARVECTOR FileForSigning;// здесь будет хранится полученная строка из ListBox
		LRESULT TextLen = SendMessageW(hListSelectedFilesForCertification, LB_GETTEXTLEN, (WPARAM)i, NULL);//получаем длинну строки
		if (TextLen != LB_ERR) {// если не ошибка
			FileForSigning.resize(TextLen + 1);//инициализируем массив ListBoxString, к TextLen прибавляем 1, т.к. длинна, которую мы получили в предыдущем шаге не включает терминирующий ноль
			if (SendMessageW(hListSelectedFilesForCertification, LB_GETTEXT, (WPARAM)i, (LPARAM)FileForSigning.data()) != LB_ERR) {//получаем текст элемента, если он был получен успешно
				SF.SigningFileName = FileForSigning.data();
				SIGN_FILE_RESULT result = SignFile(&SF, PFSF->pfSignerSign);
				if (result.InitializationFailed) {
					MessageBoxW(PFSF->hDlg, L"Ошибка инициализации функции SignFile, дальнейшую подпись файлов продолжить невозможно!", L"Ошибка инициализации функции SignFile", MB_OK | MB_ICONERROR);
					MessageBoxW(PFSF->hDlg, result.ErrorInfo.ErrorString.c_str(), result.ErrorInfo.ErrorCaption.c_str(), MB_OK | MB_ICONERROR);
					if (PS.ConsoleIsAlloced)FreeConsole();
					break;
				}
				if (!result.FileIsSigned) {
					//в случае ошибки подписи файла
					SendMessageW(hProgressForSigningFiles, PBM_STEPIT, 0, 0);
					DWORD CountWriteChars = 0;
					result.ErrorInfo.ErrorCaption = +L"[" + std::to_wstring(i + 1) + L"]" + result.ErrorInfo.ErrorCaption + L"\n";
					result.ErrorInfo.ErrorString += L"\n";
					if ((PFSF->hErrorConsole != NULL) && (PFSF->hErrorConsole != INVALID_HANDLE_VALUE)) {
						SetConsoleTextAttribute(PFSF->hErrorConsole, FOREGROUND_RED);
						WriteConsoleW(PFSF->hErrorConsole, result.ErrorInfo.ErrorCaption.c_str(), result.ErrorInfo.ErrorCaption.size(), &CountWriteChars, NULL);
						WriteConsoleW(PFSF->hErrorConsole, result.ErrorInfo.ErrorString.c_str(), result.ErrorInfo.ErrorString.size(), &CountWriteChars, NULL);
					}
					#ifdef _DEBUG
					else {
						WSTRING OutputString = result.ErrorInfo.ErrorCaption + result.ErrorInfo.ErrorString;
						OutputDebugStringW(OutputString.c_str());
					}
					#endif
					if (!AnErrorHasOccurred)AnErrorHasOccurred = true;
					continue;
				}
				else {
					//В случае успешной подписи файлов
					SendMessageW(hProgressForSigningFiles, PBM_STEPIT, 0, 0);
					DWORD CountWriteChars = 0;
					WSTRING OutputSigningFileInfo = L"[" + std::to_wstring(i + 1) + L"]";
					OutputSigningFileInfo += L"Файл \"";
					OutputSigningFileInfo += FileForSigning.data();
					OutputSigningFileInfo += L"\" был успешно подписан\n";
					if ((PFSF->hOutputConsole != NULL) && (PFSF->hOutputConsole != INVALID_HANDLE_VALUE)) {
						SetConsoleTextAttribute(PFSF->hOutputConsole, FOREGROUND_GREEN);
						WriteConsoleW(PFSF->hOutputConsole, OutputSigningFileInfo.c_str(), OutputSigningFileInfo.size(), &CountWriteChars, NULL);
					}
					#ifdef _DEBUG
					else OutputDebugStringW(OutputSigningFileInfo.c_str());
					#endif
					continue;
				}

			}
			else {
				//В случае ошибки получения имени файла
				SendMessageW(hProgressForSigningFiles, PBM_STEPIT, 0, 0);
				DWORD CountWriteChars = 0;
				WSTRING OutputSigningFileInfo = L"[" + std::to_wstring(i + 1) + L"]";
				OutputSigningFileInfo = L"Не удалось получить имя файла под индексом ";
				OutputSigningFileInfo += std::to_wstring(i);
				OutputSigningFileInfo += L" в списке, файл не был подписан\n";
				if ((PFSF->hErrorConsole != NULL) && (PFSF->hErrorConsole != INVALID_HANDLE_VALUE)) {
					SetConsoleTextAttribute(PFSF->hErrorConsole, FOREGROUND_RED);
					WriteConsoleW(PFSF->hErrorConsole, OutputSigningFileInfo.c_str(), OutputSigningFileInfo.size(), &CountWriteChars, NULL);
				}
				#ifdef _DEBUG
				else OutputDebugStringW(OutputSigningFileInfo.c_str());
				#endif
				if (!AnErrorHasOccurred) {
					AnErrorHasOccurred = true; 
					SendMessageW(hProgressForSigningFiles, PBM_SETSTATE, PBST_ERROR, 0);
				}
				continue;
			}
		}
		else {
			//В случае ошибки получения длинны имени файла
			SendMessageW(hProgressForSigningFiles, PBM_STEPIT, 0, 0);
			DWORD CountWriteChars = 0;
			WSTRING OutputSigningFileInfo = L"[" + std::to_wstring(i + 1) + L"]";
			OutputSigningFileInfo = L"Не удалось получить длинну имени файла под индексом ";
			OutputSigningFileInfo += std::to_wstring(i);
			OutputSigningFileInfo += L" в списке, файл не был подписан\n";
			if ((PFSF->hErrorConsole != NULL) && (PFSF->hErrorConsole != INVALID_HANDLE_VALUE)) {
				SetConsoleTextAttribute(PFSF->hErrorConsole, FOREGROUND_RED);
				WriteConsoleW(PFSF->hErrorConsole, OutputSigningFileInfo.c_str(), OutputSigningFileInfo.size(), &CountWriteChars, NULL);
			}
			#ifdef _DEBUG
			else OutputDebugStringW(OutputSigningFileInfo.c_str());
			#endif
			if (!AnErrorHasOccurred)AnErrorHasOccurred = true;
			continue;
		}

	}
	SetWindowTextW(hQuitCancel, L"Выход");
	HWND hConsoleWindow = (PS.ConsoleIsAlloced == TRUE ? GetConsoleWindow() : NULL);
	if (hConsoleWindow != NULL)EnableWindowClose(hConsoleWindow);
	EnableWindowClose(PFSF->hDlg);
	if(PFSF->SignCertificate != NULL)CertFreeCertificateContext(PFSF->SignCertificate);
	if(PFSF->hCertStore != NULL)CertCloseStore(PFSF->hCertStore, CERT_CLOSE_STORE_FORCE_FLAG);
	if (PFSF->hStatusBar != NULL) {
		if (AnErrorHasOccurred)SendMessageW(PFSF->hStatusBar, SB_SETTEXTW, MAKEWORD(0, SBT_POPOUT), (LPARAM)L"Подпись завершена с ошибками");
		else SendMessageW(PFSF->hStatusBar, SB_SETTEXTW, MAKEWORD(0, SBT_POPOUT), (LPARAM)L"Подпись файлов завершена");
	}
	if (AnErrorHasOccurred)SendMessageW(hProgressForSigningFiles, PBM_SETSTATE, PBST_ERROR, 0);
	//включение основных элементов управления, которые до начала подписи были отключены
	EnableWindow(hSign, TRUE);
	EnableWindow(hClear, TRUE);
	//возвращаем предыдущее состояние кнопок удалить и изменить
	if (ButtonDeleteFileLastStatus == NULL)EnableWindow(hDeleteFile, TRUE);
	if (ButtonModifyFileLastStatus == NULL)EnableWindow(hModifyFile, TRUE);
	//конец возвращения предыдущего состояния
	EnableWindow(hAddFiles, TRUE);
	// отключение элементов управления, которые до начала подписи были включены
	EnableWindow(hPauseSigning, FALSE);
	if (PFSF->hMssign32 != NULL)FreeLibrary(PFSF->hMssign32);
	return ERROR_SUCCESS;
}
bool CheckBit(unsigned word, unsigned bit) {// функция проверяет установлен ли бит под определённым номером
	return (word & (1u << bit)) != 0;
}
bool FileReadebleExists(LPCTSTR fname)
{
	HANDLE hFile = CreateFileW(fname, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (INVALID_HANDLE_VALUE != hFile){
		CloseHandle(hFile);
		return true;
	}
	return false;
}
void ProgrammSettings::LoadProgrammSettingsOfRegistry(unsigned sfLoad) {//данная функция предназначена для загрузки параметров программы из реестра Windows
	HKEY hMainRootKey = NULL;
	LSTATUS RegOpenKeyResult = RegOpenKeyExW(HKEY_CURRENT_USER, ROOT_REGISTRY_KEY_SIGNPROJECT_TOOL, 0, KEY_READ, &hMainRootKey);
	if (RegOpenKeyResult == ERROR_SUCCESS) {
		//Блок получения параметра CertificateFile
		if (CheckBit(sfLoad, 0)) {
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
		if (CheckBit(sfLoad, 1)) {
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
		int ResultQuestion = MessageBoxW(PS.hRootWnd, L"Создать данный ключ?", L"Требуется ваше вмешательство", MB_YESNO | MB_ICONQUESTION);
		switch (ResultQuestion) {
		case IDYES:
			SaveProgrammSettingsInRegistry(SIGN_CERTIFICATE_FILE_NAME | ALG_HASH);
			break;
		case IDNO:
		case IDCANCEL:
			break;
		}
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
					if (CheckBit(sfSave, 0)) {
						LSTATUS RegSetValueExResult = RegSetValueExW(hMainRootKey, L"CertificateFile", 0, REG_EXPAND_SZ, (BYTE*)this->CertificateFile.c_str(), (this->CertificateFile.size() + 1) * sizeof(WCHAR));
						if (RegSetValueExResult != ERROR_SUCCESS)MessageError(L"Не удалось создать параметр CertificateFile в реестре по пути HKEY_CURRENT_USER\\" ROOT_REGISTRY_KEY_SIGNPROJECT_TOOL, L"Ошибка создания параметра!", PS.hRootWnd, RegSetValueExResult);
					}
					if (CheckBit(sfSave, 1)) {
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