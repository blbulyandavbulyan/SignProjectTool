#pragma once
/*******************************************************************************************************
****В этом заголовочном файле содержатся все функции которые необходимы программе для подписи файлов****
********************************************************************************************************/
//cтруктуры, необходимые для цифровой подписи файлов
typedef struct _SIGNER_FILE_INFO {
	DWORD cbSize;
	LPCWSTR pwszFileName;
	HANDLE hFile;
} SIGNER_FILE_INFO, * PSIGNER_FILE_INFO;

typedef struct _SIGNER_BLOB_INFO {
	DWORD cbSize;
	GUID* pGuidSubject;
	DWORD cbBlob;
	BYTE* pbBlob;
	LPCWSTR pwszDisplayName;
} SIGNER_BLOB_INFO, * PSIGNER_BLOB_INFO;

typedef struct _SIGNER_SUBJECT_INFO {
	DWORD cbSize;
	DWORD* pdwIndex;
	DWORD dwSubjectChoice;
	union {
		SIGNER_FILE_INFO* pSignerFileInfo;
		SIGNER_BLOB_INFO* pSignerBlobInfo;
	};
} SIGNER_SUBJECT_INFO, * PSIGNER_SUBJECT_INFO;

typedef struct _SIGNER_CERT_STORE_INFO {
	DWORD cbSize;
	PCCERT_CONTEXT pSigningCert;
	DWORD dwCertPolicy;
	HCERTSTORE hCertStore;
} SIGNER_CERT_STORE_INFO, * PSIGNER_CERT_STORE_INFO;

typedef struct _SIGNER_SPC_CHAIN_INFO {
	DWORD cbSize;
	LPCWSTR pwszSpcFile;
	DWORD dwCertPolicy;
	HCERTSTORE hCertStore;
} SIGNER_SPC_CHAIN_INFO, * PSIGNER_SPC_CHAIN_INFO;

typedef struct _SIGNER_CERT {
	DWORD cbSize;
	DWORD dwCertChoice;
	union {
		LPCWSTR pwszSpcFile;
		SIGNER_CERT_STORE_INFO* pCertStoreInfo;
		SIGNER_SPC_CHAIN_INFO* pSpcChainInfo;
	};
	HWND hwnd;
} SIGNER_CERT, * PSIGNER_CERT;

typedef struct _SIGNER_ATTR_AUTHCODE {
	DWORD cbSize;
	BOOL fCommercial;
	BOOL fIndividual;
	LPCWSTR pwszName;
	LPCWSTR pwszInfo;
} SIGNER_ATTR_AUTHCODE, * PSIGNER_ATTR_AUTHCODE;

typedef struct _SIGNER_SIGNATURE_INFO {
	DWORD cbSize;
	ALG_ID algidHash;
	DWORD dwAttrChoice;
	union {
		SIGNER_ATTR_AUTHCODE* pAttrAuthcode;
	};
	PCRYPT_ATTRIBUTES psAuthenticated;
	PCRYPT_ATTRIBUTES psUnauthenticated;
} SIGNER_SIGNATURE_INFO, * PSIGNER_SIGNATURE_INFO;

typedef struct _SIGNER_PROVIDER_INFO {
	DWORD cbSize;
	LPCWSTR pwszProviderName;
	DWORD dwProviderType;
	DWORD dwKeySpec;
	DWORD dwPvkChoice;
	union {
		LPWSTR pwszPvkFileName;
		LPWSTR pwszKeyContainer;
	};
} SIGNER_PROVIDER_INFO, * PSIGNER_PROVIDER_INFO;
typedef struct _SIGNER_CONTEXT {
	DWORD cbSize;
	DWORD cbBlob;
	BYTE* pbBlob;
} SIGNER_CONTEXT, * PSIGNER_CONTEXT;
//необходимые функции для цифровой подписи файлов
typedef HRESULT(WINAPI* SignerSignType)(
	_In_     SIGNER_SUBJECT_INFO* pSubjectInfo,
	_In_     SIGNER_CERT* pSignerCert,
	_In_     SIGNER_SIGNATURE_INFO* pSignatureInfo,
	_In_opt_ SIGNER_PROVIDER_INFO* pProviderInfo,
	_In_opt_ LPCWSTR               pwszHttpTimeStamp,
	_In_opt_ PCRYPT_ATTRIBUTES     psRequest,
	_In_opt_ LPVOID                pSipData);
//typedef HANDLE(WINAPI* PGetProcessHandleFromHwnd)(_In_ HWND hWnd);
//всё что связано с процедурой подписи живёт тут
struct SIGN_FILE_RESULT {//данную структуру возвращает функция SignFile
	struct SIGNING_FILE {//данная структура передаётся функции  SignFile в качестве параметра
		PCCERT_CONTEXT SignCertificate = nullptr;
		LPCWSTR SigningFileName = nullptr;//имя подписываемого файла
		ALG_ID HashAlgorithmId = DEFUAULT_SIGN_HASH_ALGORITHM;// алгоритм хеширования для цифровой подписи
		DWORD dwAttrChoice = 0;//требуется ли использовать pAttrAuthcode
		SIGNER_ATTR_AUTHCODE* pAttrAuthcode = nullptr;//дополнительные параметры для подписи
	};
	typedef const SIGNING_FILE* PCSIGNING_FILE;
	bool FileIsSigned = false;//файл был подписан
	bool InitializationFailed = false;//была ли ошибка инициализации функции SignFile
	WWS::WWS_ERROR_REPORT_STRUCTW ErrorInfo;//данный член будет заполнен в том случае, если не удастся инициализировать функцию SIGN_FILES_RESULT
};
//структура с параметрами для потока, выполняющего подпись файлов
struct PARAMETERS_FOR_THREAD_SIGN_FILES {
	enum THREAD_MESSAGES { THM_CANCEL, THM_CLOSE };//перечисление с основными сообщениями для потока
	/*
		THM_CANCEL - посылается в случае отмены операции подписи файлов
		THM_CLOSE - посылается в случае закрытия приложения
	*/
	HWND hDlg = NULL;//дескриптор диалога, относительно которого будут выводится сообщения
	HWND hStatusBar = NULL;//дескриптор окна статус бара, в который выводится информация о ходе подписи
	HANDLE hOutputConsole = nullptr;//дескриптор вывода консоли
	HANDLE hErrorConsole = nullptr;//дескриптор вывода ошибок консоли
	BOOL ConsoleIsAlloced = FALSE;// была ли выделена консоль
	HMODULE hMssign32 = NULL;//дескриптор библиотеки Mssign32.dll
	LRESULT ItemsCount = 0;//количество элементов в списке hListSelectedFilesForCertification
	PCCERT_CONTEXT SignCertificate = nullptr;//сертификат для подписи
	HCERTSTORE hCertStore = nullptr;//хранилище сертификатов, в котором хранится сертификат SignCertificate
	SignerSignType pfSignerSign = nullptr;// указатель на функциюю SignerSign
	byte CommandToExecuteAfterSigningFiles = 0b00000000;//команда для исполнения после подписи файлов(детали описаны в комментарии в структуре с основными параметрами программы)
	
	typedef PARAMETERS_FOR_THREAD_SIGN_FILES* PPARAMETERS_FOR_THREAD_SIGN_FILES;
};
struct LoadCertificateResult {//данную структуру возвращает функция LoadCertificate
	bool CertificateIsLoaded = false;//сертификат был загружен
	PCCERT_CONTEXT pCertContext = nullptr;//структура, содержащая загруженный сертификат
	HCERTSTORE hCertStore = nullptr;
	WWS::WWS_ERROR_REPORT_STRUCTW ErrorInfo;//в случае если сертификат не был загружен здесь содержится информация об ошибке
};
DWORD WINAPI SignFilesThreadProc(_In_ LPVOID lpParameter);//поточная процедура, отвечающая за подпись файлов
SIGN_FILE_RESULT SignFile(SIGN_FILE_RESULT::PCSIGNING_FILE SF, SignerSignType pfSignerSign);//функция предназначена для подписи файла
LoadCertificateResult LoadCertificate(HWND hWnd, const WSTRING* CertificateHref, bool LoadCertificateFromCertStore, LPCWSTR password);//функция предназначена для загрузки сертификата из файла/хранилища(загрузка из хранилища не реализована)