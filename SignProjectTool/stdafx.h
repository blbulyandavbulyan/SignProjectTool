// stdafx.h: включаемый файл для стандартных системных включаемых файлов
// или включаемых файлов для конкретного проекта, которые часто используются, но
// нечасто изменяются
//Этот файл относится к проекту SignProjectTool

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Исключите редко используемые компоненты из заголовков Windows
// Файлы заголовков Windows
#include <windows.h>
#include <ShObjIdl.h>
#include <wincrypt.h>
#include <cryptuiapi.h>
#include <Uxtheme.h>
// Файлы заголовков среды выполнения C
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <iostream>
//Файлы заголовков проектных DLL
#include "../WWS/WWS.h"
#define BDL_ENABLE_PROTOTYPES
#include "../BDL/BDL.h"
#include "../types.h"
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

//cтруктуры, необходимые для цифровой подписи файлов
typedef struct _SIGNER_FILE_INFO {
	DWORD cbSize;
	LPCWSTR pwszFileName;
	HANDLE hFile;
} SIGNER_FILE_INFO, *PSIGNER_FILE_INFO;

typedef struct _SIGNER_BLOB_INFO {
	DWORD cbSize;
	GUID *pGuidSubject;
	DWORD cbBlob;
	BYTE *pbBlob;
	LPCWSTR pwszDisplayName;
} SIGNER_BLOB_INFO, *PSIGNER_BLOB_INFO;

typedef struct _SIGNER_SUBJECT_INFO {
	DWORD cbSize;
	DWORD *pdwIndex;
	DWORD dwSubjectChoice;
	union {
		SIGNER_FILE_INFO *pSignerFileInfo;
		SIGNER_BLOB_INFO *pSignerBlobInfo;
	};
} SIGNER_SUBJECT_INFO, *PSIGNER_SUBJECT_INFO;

typedef struct _SIGNER_CERT_STORE_INFO {
	DWORD cbSize;
	PCCERT_CONTEXT pSigningCert;
	DWORD dwCertPolicy;
	HCERTSTORE hCertStore;
} SIGNER_CERT_STORE_INFO, *PSIGNER_CERT_STORE_INFO;

typedef struct _SIGNER_SPC_CHAIN_INFO {
	DWORD cbSize;
	LPCWSTR pwszSpcFile;
	DWORD dwCertPolicy;
	HCERTSTORE hCertStore;
} SIGNER_SPC_CHAIN_INFO, *PSIGNER_SPC_CHAIN_INFO;

typedef struct _SIGNER_CERT {
	DWORD cbSize;
	DWORD dwCertChoice;
	union {
		LPCWSTR pwszSpcFile;
		SIGNER_CERT_STORE_INFO *pCertStoreInfo;
		SIGNER_SPC_CHAIN_INFO *pSpcChainInfo;
	};
	HWND hwnd;
} SIGNER_CERT, *PSIGNER_CERT;

typedef struct _SIGNER_ATTR_AUTHCODE {
	DWORD cbSize;
	BOOL fCommercial;
	BOOL fIndividual;
	LPCWSTR pwszName;
	LPCWSTR pwszInfo;
} SIGNER_ATTR_AUTHCODE, *PSIGNER_ATTR_AUTHCODE;

typedef struct _SIGNER_SIGNATURE_INFO {
	DWORD cbSize;
	ALG_ID algidHash;
	DWORD dwAttrChoice;
	union {
		SIGNER_ATTR_AUTHCODE *pAttrAuthcode;
	};
	PCRYPT_ATTRIBUTES psAuthenticated;
	PCRYPT_ATTRIBUTES psUnauthenticated;
} SIGNER_SIGNATURE_INFO, *PSIGNER_SIGNATURE_INFO;

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
} SIGNER_PROVIDER_INFO, *PSIGNER_PROVIDER_INFO;
typedef struct _SIGNER_CONTEXT {
	DWORD cbSize;
	DWORD cbBlob;
	BYTE *pbBlob;
} SIGNER_CONTEXT, *PSIGNER_CONTEXT;
//необходимые функции для цифровой подписи файлов
typedef HRESULT(WINAPI *SignerSignType)(
	_In_     SIGNER_SUBJECT_INFO   *pSubjectInfo,
	_In_     SIGNER_CERT           *pSignerCert,
	_In_     SIGNER_SIGNATURE_INFO *pSignatureInfo,
	_In_opt_ SIGNER_PROVIDER_INFO  *pProviderInfo,
	_In_opt_ LPCWSTR               pwszHttpTimeStamp,
	_In_opt_ PCRYPT_ATTRIBUTES     psRequest,
	_In_opt_ LPVOID                pSipData);
// установите здесь ссылки на дополнительные заголовки, требующиеся для программы
typedef HANDLE(WINAPI* PGetProcessHandleFromHwnd)(_In_ HWND hWnd);