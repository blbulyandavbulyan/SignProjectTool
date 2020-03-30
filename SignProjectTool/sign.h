#pragma once
/*******************************************************************************************************
****� ���� ������������ ����� ���������� ��� ������� ������� ���������� ��������� ��� ������� ������****
********************************************************************************************************/
//c��������, ����������� ��� �������� ������� ������
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
//����������� ������� ��� �������� ������� ������
typedef HRESULT(WINAPI* SignerSignType)(
	_In_     SIGNER_SUBJECT_INFO* pSubjectInfo,
	_In_     SIGNER_CERT* pSignerCert,
	_In_     SIGNER_SIGNATURE_INFO* pSignatureInfo,
	_In_opt_ SIGNER_PROVIDER_INFO* pProviderInfo,
	_In_opt_ LPCWSTR               pwszHttpTimeStamp,
	_In_opt_ PCRYPT_ATTRIBUTES     psRequest,
	_In_opt_ LPVOID                pSipData);
//typedef HANDLE(WINAPI* PGetProcessHandleFromHwnd)(_In_ HWND hWnd);
//�� ��� ������� � ���������� ������� ���� ���
struct SIGN_FILE_RESULT {//������ ��������� ���������� ������� SignFile
	struct SIGNING_FILE {//������ ��������� ��������� �������  SignFile � �������� ���������
		PCCERT_CONTEXT SignCertificate = nullptr;
		LPCWSTR SigningFileName = nullptr;//��� �������������� �����
		ALG_ID HashAlgorithmId = DEFUAULT_SIGN_HASH_ALGORITHM;// �������� ����������� ��� �������� �������
		DWORD dwAttrChoice = 0;//��������� �� ������������ pAttrAuthcode
		SIGNER_ATTR_AUTHCODE* pAttrAuthcode = nullptr;//�������������� ��������� ��� �������
	};
	typedef const SIGNING_FILE* PCSIGNING_FILE;
	bool FileIsSigned = false;//���� ��� ��������
	bool InitializationFailed = false;//���� �� ������ ������������� ������� SignFile
	WWS::WWS_ERROR_REPORT_STRUCTW ErrorInfo;//������ ���� ����� �������� � ��� ������, ���� �� ������� ���������������� ������� SIGN_FILES_RESULT
};
//��������� � ����������� ��� ������, ������������ ������� ������
struct PARAMETERS_FOR_THREAD_SIGN_FILES {
	enum THREAD_MESSAGES { THM_CANCEL, THM_CLOSE };//������������ � ��������� ����������� ��� ������
	/*
		THM_CANCEL - ���������� � ������ ������ �������� ������� ������
		THM_CLOSE - ���������� � ������ �������� ����������
	*/
	HWND hDlg = NULL;//���������� �������, ������������ �������� ����� ��������� ���������
	HWND hStatusBar = NULL;//���������� ���� ������ ����, � ������� ��������� ���������� � ���� �������
	HANDLE hOutputConsole = nullptr;//���������� ������ �������
	HANDLE hErrorConsole = nullptr;//���������� ������ ������ �������
	BOOL ConsoleIsAlloced = FALSE;// ���� �� �������� �������
	HMODULE hMssign32 = NULL;//���������� ���������� Mssign32.dll
	LRESULT ItemsCount = 0;//���������� ��������� � ������ hListSelectedFilesForCertification
	PCCERT_CONTEXT SignCertificate = nullptr;//���������� ��� �������
	HCERTSTORE hCertStore = nullptr;//��������� ������������, � ������� �������� ���������� SignCertificate
	SignerSignType pfSignerSign = nullptr;// ��������� �� �������� SignerSign
	byte CommandToExecuteAfterSigningFiles = 0b00000000;//������� ��� ���������� ����� ������� ������(������ ������� � ����������� � ��������� � ��������� ����������� ���������)
	
	typedef PARAMETERS_FOR_THREAD_SIGN_FILES* PPARAMETERS_FOR_THREAD_SIGN_FILES;
};
struct LoadCertificateResult {//������ ��������� ���������� ������� LoadCertificate
	bool CertificateIsLoaded = false;//���������� ��� ��������
	PCCERT_CONTEXT pCertContext = nullptr;//���������, ���������� ����������� ����������
	HCERTSTORE hCertStore = nullptr;
	WWS::WWS_ERROR_REPORT_STRUCTW ErrorInfo;//� ������ ���� ���������� �� ��� �������� ����� ���������� ���������� �� ������
};
DWORD WINAPI SignFilesThreadProc(_In_ LPVOID lpParameter);//�������� ���������, ���������� �� ������� ������
SIGN_FILE_RESULT SignFile(SIGN_FILE_RESULT::PCSIGNING_FILE SF, SignerSignType pfSignerSign);//������� ������������� ��� ������� �����
LoadCertificateResult LoadCertificate(HWND hWnd, const WSTRING* CertificateHref, bool LoadCertificateFromCertStore, LPCWSTR password);//������� ������������� ��� �������� ����������� �� �����/���������(�������� �� ��������� �� �����������)