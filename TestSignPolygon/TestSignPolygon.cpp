// TestSignPolygon.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include "stdafx.h"
HRESULT SignAppxPackage(_In_ PCCERT_CONTEXT signingCertContext, _In_ LPCWSTR packageFilePath);
LoadCertificateResult LoadCertificate(HWND hWnd, const WSTRING* CertificateHref, bool LoadCertificateFromCertStore, LPCWSTR password);
int main()
{
	WSTRING CertificateHref = L"C:\\Users\\blbul\\OneDrive\\Документы\\Сертификаты\\CyberForum.pfx";
	LoadCertificateResult LCR = LoadCertificate(NULL, &CertificateHref, false, L"");
	if (LCR.CertificateIsLoaded) {
		HRESULT hr = SignAppxPackage(LCR.pCertContext, L"C:\\Users\\blbul\\OneDrive\\Рабочий стол\\For Signing\\explorer.exe");
		CertFreeCertificateContext(LCR.pCertContext);
		CertFreeCertificateContext(LCR.pCertContext);
		if (SUCCEEDED(hr)) {
			std::cout << "Signing is okay!\n";
			return 0;
		}
		else {
			std::cerr << "Signing is fail!\n";
			MessageError(L"Произошла ошибка при подписи файла!", L"Ошибка подписи!", NULL, hr);
			return -1;
		}
	}
	else {
		std::cout << "ERROR! CERTIFICATE IS NOT LOADED!\n";
		return -1;
	}
    return 0;
}
LoadCertificateResult LoadCertificate(HWND hWnd, const WSTRING* CertificateHref, bool LoadCertificateFromCertStore, LPCWSTR password) {
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
HRESULT SignAppxPackage(
	_In_ PCCERT_CONTEXT signingCertContext,
	_In_ LPCWSTR packageFilePath)
{
	HRESULT hr = S_OK;

	// Initialize the parameters for SignerSignEx2
	DWORD signerIndex = 0;

	SIGNER_FILE_INFO fileInfo = {};
	fileInfo.cbSize = sizeof(SIGNER_FILE_INFO);
	fileInfo.pwszFileName = packageFilePath;

	SIGNER_SUBJECT_INFO subjectInfo = {};
	subjectInfo.cbSize = sizeof(SIGNER_SUBJECT_INFO);
	subjectInfo.pdwIndex = &signerIndex;
	subjectInfo.dwSubjectChoice = SIGNER_SUBJECT_FILE;
	subjectInfo.pSignerFileInfo = &fileInfo;

	SIGNER_CERT_STORE_INFO certStoreInfo = {};
	certStoreInfo.cbSize = sizeof(SIGNER_CERT_STORE_INFO);
	certStoreInfo.dwCertPolicy = SIGNER_CERT_POLICY_CHAIN_NO_ROOT;
	certStoreInfo.pSigningCert = signingCertContext;

	SIGNER_CERT cert = {};
	cert.cbSize = sizeof(SIGNER_CERT);
	cert.dwCertChoice = SIGNER_CERT_STORE;
	cert.pCertStoreInfo = &certStoreInfo;

	// The algidHash of the signature to be created must match the
	// hash algorithm used to create the app package
	SIGNER_SIGNATURE_INFO signatureInfo = {};
	signatureInfo.cbSize = sizeof(SIGNER_SIGNATURE_INFO);
	signatureInfo.algidHash = CALG_SHA_256;
	signatureInfo.dwAttrChoice = SIGNER_NO_ATTR;

	SIGNER_SIGN_EX2_PARAMS signerParams = {};
	signerParams.pSubjectInfo = &subjectInfo;
	signerParams.pSigningCert = &cert;
	signerParams.pSignatureInfo = &signatureInfo;

	APPX_SIP_CLIENT_DATA sipClientData = {};
	sipClientData.pSignerParams = &signerParams;
	signerParams.pSipData = &sipClientData;

	// Type definition for invoking SignerSignEx2 via GetProcAddress
	typedef HRESULT(WINAPI * SignerSignEx2Function)(
		DWORD,
		PSIGNER_SUBJECT_INFO,
		PSIGNER_CERT,
		PSIGNER_SIGNATURE_INFO,
		PSIGNER_PROVIDER_INFO,
		DWORD,
		PCSTR,
		PCWSTR,
		PCRYPT_ATTRIBUTES,
		PVOID,
		PSIGNER_CONTEXT*,
		PVOID,
		PVOID);

	// Load the SignerSignEx2 function from MSSign32.dll
	HMODULE msSignModule = LoadLibraryEx(
		L"MSSign32.dll",
		NULL,
		LOAD_LIBRARY_SEARCH_SYSTEM32);

	if (msSignModule)
	{
		SignerSignEx2Function SignerSignEx2 = reinterpret_cast<SignerSignEx2Function>(
			GetProcAddress(msSignModule, "SignerSignEx2"));
		if (SignerSignEx2)
		{
			hr = SignerSignEx2(
				signerParams.dwFlags,
				signerParams.pSubjectInfo,
				signerParams.pSigningCert,
				signerParams.pSignatureInfo,
				signerParams.pProviderInfo,
				signerParams.dwTimestampFlags,
				signerParams.pszAlgorithmOid,
				signerParams.pwszTimestampURL,
				signerParams.pCryptAttrs,
				signerParams.pSipData,
				signerParams.pSignerContext,
				signerParams.pCryptoPolicy,
				signerParams.pReserved);
		}
		else
		{
			DWORD lastError = GetLastError();
			hr = HRESULT_FROM_WIN32(lastError);
		}

		FreeLibrary(msSignModule);
	}
	else
	{
		DWORD lastError = GetLastError();
		hr = HRESULT_FROM_WIN32(lastError);
	}

	// Free any state used during app package signing
	if (sipClientData.pAppxSipState)
	{
		sipClientData.pAppxSipState->Release();
	}

	return hr;
}
// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.
