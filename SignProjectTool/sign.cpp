#include "stdafx.h"
/**************************************************************
****����� ��������� ���������� ������� ����������� � sign.h****
***************************************************************/
//�������� ���������, ���������� �� ������� ������

/*���� ������� �������������� ������ � ���� �����*/
//������� ��������� ������ �������� �� ����
void DisableWindowClose(HWND hwnd) {
	// ��������� ������ �������.
	SetClassLongPtr(hwnd, GCL_STYLE, GetClassLongPtr(hwnd, GCL_STYLE) | CS_NOCLOSE);
	// ��������� ��������� ���� �����.
	EnableMenuItem(GetSystemMenu(hwnd, FALSE), SC_CLOSE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
}
//������� �������� ������ �������� �� ����
void EnableWindowClose(HWND hwnd) {
	// �������� ������ �������.
	SetClassLongPtr(hwnd, GCL_STYLE, GetClassLongPtr(hwnd, GCL_STYLE) & ~CS_NOCLOSE);
	// �������� ����� ���������� ����.
	EnableMenuItem(GetSystemMenu(hwnd, FALSE), SC_CLOSE, MF_BYCOMMAND | MF_ENABLED);
}
/*����� ����� ������� �������������� ������ � ���� �����*/

//�������� ��������� ����������� ������� ������
DWORD WINAPI SignFilesThreadProc(_In_ LPVOID lpParameter) {
	PARAMETERS_FOR_THREAD_SIGN_FILES::PPARAMETERS_FOR_THREAD_SIGN_FILES PFSF = (PARAMETERS_FOR_THREAD_SIGN_FILES::PPARAMETERS_FOR_THREAD_SIGN_FILES)lpParameter;
	HWND hListSelectedFilesForCertification = GetDlgItem(PFSF->hDlg, IDC_LIST_SELECTED_FILES_FOR_CERTIFICTION), hDeleteFile = GetDlgItem(PFSF->hDlg, IDC_DELETE_FILE), hModifyFile = GetDlgItem(PFSF->hDlg, IDC_MODIFY_FILE), hSign = GetDlgItem(PFSF->hDlg, IDC_SIGN), hClear = GetDlgItem(PFSF->hDlg, IDC_CLEAR), hQuitCancel = GetDlgItem(PFSF->hDlg, IDCANCEL), hProgressForSigningFiles = GetDlgItem(PFSF->hDlg, IDC_PROGRESS_SIGNING_FILES), hAddFiles = GetDlgItem(PFSF->hDlg, IDC_ADD_FILE), hPauseSigning = GetDlgItem(PFSF->hDlg, IDC_PAUSE_SIGNING);
	SIGN_FILE_RESULT::SIGNING_FILE SF;
	BOOL ButtonModifyFileLastStatus, ButtonDeleteFileLastStatus;
	BOOL AnErrorHasOccurred = FALSE;
	HWND hConsoleWindow = (PFSF->ConsoleIsAlloced == TRUE ? GetConsoleWindow() : NULL);
	SF.SignCertificate = PFSF->SignCertificate;
	MSG msg;
	//���������� ������������� ���������
	SetWindowTextW(hQuitCancel, L"������");
	SendMessageW(hProgressForSigningFiles, PBM_SETRANGE32, 0, PFSF->ItemsCount);
	SendMessageW(hProgressForSigningFiles, PBM_SETPOS, 0, 0);
	SendMessageW(hProgressForSigningFiles, PBM_SETSTEP, (WPARAM)1, NULL);
	SendMessageW(PFSF->hStatusBar, SB_SETTEXTW, MAKEWORD(0, SBT_POPOUT), (LPARAM)L"��� ������� ������");
	//���������� ����������� ��������� ���������� �� ����� �������
	EnableWindow(hSign, FALSE);
	EnableWindow(hClear, FALSE);
	ButtonModifyFileLastStatus = EnableWindow(hModifyFile, FALSE);
	ButtonDeleteFileLastStatus = EnableWindow(hDeleteFile, FALSE);
	EnableWindow(hAddFiles, FALSE);
	//��������� ����������� ��������� �� ����� �������
	EnableWindow(hPauseSigning, TRUE);
	if (hConsoleWindow != NULL)DisableWindowClose(hConsoleWindow);
	for (LRESULT i = 0; i < PFSF->ItemsCount; i++) {//���� ������� ������
		if (PeekMessageW(&msg, NULL, PARAMETERS_FOR_THREAD_SIGN_FILES::THREAD_MESSAGES::THM_CANCEL, PARAMETERS_FOR_THREAD_SIGN_FILES::THREAD_MESSAGES::THM_CLOSE, PM_REMOVE)) {
			switch (msg.message) {
			case PARAMETERS_FOR_THREAD_SIGN_FILES::THREAD_MESSAGES::THM_CLOSE://������ ��������� ��������� ��� ������� �� ������ ��������
				if (PFSF->SignCertificate != NULL)CertFreeCertificateContext(PFSF->SignCertificate);
				if (PFSF->hCertStore != NULL)CertCloseStore(PFSF->hCertStore, CERT_CLOSE_STORE_FORCE_FLAG);
				if (PFSF->hStatusBar != NULL)SendMessageW(PFSF->hStatusBar, SB_SETTEXTW, MAKEWORD(0, SBT_POPOUT), (LPARAM)L"���������� ����������");
				if (PFSF->hMssign32 != NULL)FreeLibrary(PFSF->hMssign32);
				return 0;
			case PARAMETERS_FOR_THREAD_SIGN_FILES::THREAD_MESSAGES::THM_CANCEL: {//������ ��������� ��������� ��� ������� �� ������ ������
				SetWindowTextW(hQuitCancel, L"�����");
				if (hConsoleWindow != NULL)EnableWindowClose(hConsoleWindow);
				EnableWindowClose(PFSF->hDlg);
				if (PFSF->SignCertificate != NULL)CertFreeCertificateContext(PFSF->SignCertificate);
				if (PFSF->hCertStore != NULL)CertCloseStore(PFSF->hCertStore, CERT_CLOSE_STORE_FORCE_FLAG);
				if (PFSF->hStatusBar != NULL)SendMessageW(PFSF->hStatusBar, SB_SETTEXTW, MAKEWORD(0, SBT_POPOUT), (LPARAM)L"������� ��������");
				if (PFSF->ConsoleIsAlloced == TRUE)FreeConsole();
				SendMessageW(hProgressForSigningFiles, PBM_SETPOS, 0, 0);
				EnableWindow(hSign, TRUE);
				EnableWindow(hClear, TRUE);
				//���������� ���������� ��������� ������ ������� � ��������
				if (ButtonDeleteFileLastStatus == NULL)EnableWindow(hDeleteFile, TRUE);
				if (ButtonModifyFileLastStatus == NULL)EnableWindow(hModifyFile, TRUE);
				//����� ����������� ����������� ���������
				EnableWindow(hAddFiles, TRUE);
				EnableWindow(hPauseSigning, FALSE);
				if (PFSF->hMssign32 != NULL)FreeLibrary(PFSF->hMssign32);
				return ERROR_CANCELLED;
			}
			}
		}
		WCHARVECTOR FileForSigning;// ����� ����� �������� ���������� ������ �� ListBox
		LRESULT TextLen = SendMessageW(hListSelectedFilesForCertification, LB_GETTEXTLEN, (WPARAM)i, NULL);//�������� ������ ������
		if (TextLen != LB_ERR) {// ���� �� ������
			FileForSigning.resize(TextLen + 1);//�������������� ������ ListBoxString, � TextLen ���������� 1, �.�. ������, ������� �� �������� � ���������� ���� �� �������� ������������� ����
			if (SendMessageW(hListSelectedFilesForCertification, LB_GETTEXT, (WPARAM)i, (LPARAM)FileForSigning.data()) != LB_ERR) {//�������� ����� ��������, ���� �� ��� ������� �������
				SF.SigningFileName = FileForSigning.data();
				SIGN_FILE_RESULT result = SignFile(&SF, PFSF->pfSignerSign);
				if (result.InitializationFailed) {
					MessageBoxW(PFSF->hDlg, L"������ ������������� ������� SignFile, ���������� ������� ������ ���������� ����������!", L"������ ������������� ������� SignFile", MB_OK | MB_ICONERROR);
					MessageBoxW(PFSF->hDlg, result.ErrorInfo.ErrorString.c_str(), result.ErrorInfo.ErrorCaption.c_str(), MB_OK | MB_ICONERROR);
					if (PFSF->ConsoleIsAlloced)FreeConsole();
					break;
				}
				if (!result.FileIsSigned) {
					//� ������ ������ ������� �����
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
					//� ������ �������� ������� ������
					SendMessageW(hProgressForSigningFiles, PBM_STEPIT, 0, 0);
					DWORD CountWriteChars = 0;
					WSTRING OutputSigningFileInfo = L"[" + std::to_wstring(i + 1) + L"]";
					OutputSigningFileInfo += L"���� \"";
					OutputSigningFileInfo += FileForSigning.data();
					OutputSigningFileInfo += L"\" ��� ������� ��������\n";
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
				//� ������ ������ ��������� ����� �����
				SendMessageW(hProgressForSigningFiles, PBM_STEPIT, 0, 0);
				DWORD CountWriteChars = 0;
				WSTRING OutputSigningFileInfo = L"[" + std::to_wstring(i + 1) + L"]";
				OutputSigningFileInfo = L"�� ������� �������� ��� ����� ��� �������� ";
				OutputSigningFileInfo += std::to_wstring(i);
				OutputSigningFileInfo += L" � ������, ���� �� ��� ��������\n";
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
			//� ������ ������ ��������� ������ ����� �����
			SendMessageW(hProgressForSigningFiles, PBM_STEPIT, 0, 0);
			DWORD CountWriteChars = 0;
			WSTRING OutputSigningFileInfo = L"[" + std::to_wstring(i + 1) + L"]";
			OutputSigningFileInfo = L"�� ������� �������� ������ ����� ����� ��� �������� ";
			OutputSigningFileInfo += std::to_wstring(i);
			OutputSigningFileInfo += L" � ������, ���� �� ��� ��������\n";
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
	SetWindowTextW(hQuitCancel, L"�����");
	if (hConsoleWindow != NULL)EnableWindowClose(hConsoleWindow);
	EnableWindowClose(PFSF->hDlg);
	if (PFSF->SignCertificate != NULL)CertFreeCertificateContext(PFSF->SignCertificate);
	if (PFSF->hCertStore != NULL)CertCloseStore(PFSF->hCertStore, CERT_CLOSE_STORE_FORCE_FLAG);
	if (PFSF->hStatusBar != NULL) {
		if (AnErrorHasOccurred)SendMessageW(PFSF->hStatusBar, SB_SETTEXTW, MAKEWORD(0, SBT_POPOUT), (LPARAM)L"������� ��������� � ��������");
		else SendMessageW(PFSF->hStatusBar, SB_SETTEXTW, MAKEWORD(0, SBT_POPOUT), (LPARAM)L"������� ������ ���������");
	}
	if (AnErrorHasOccurred)SendMessageW(hProgressForSigningFiles, PBM_SETSTATE, PBST_ERROR, 0);
	//��������� �������� ��������� ����������, ������� �� ������ ������� ���� ���������
	EnableWindow(hSign, TRUE);
	EnableWindow(hClear, TRUE);
	//���������� ���������� ��������� ������ ������� � ��������
	if (ButtonDeleteFileLastStatus == NULL)EnableWindow(hDeleteFile, TRUE);
	if (ButtonModifyFileLastStatus == NULL)EnableWindow(hModifyFile, TRUE);
	//����� ����������� ����������� ���������
	EnableWindow(hAddFiles, TRUE);
	// ���������� ��������� ����������, ������� �� ������ ������� ���� ��������
	EnableWindow(hPauseSigning, FALSE);
	if (PFSF->hMssign32 != NULL)FreeLibrary(PFSF->hMssign32);
	return ERROR_SUCCESS;
}
/*
	������ ������� ����������� ���� ������������, ��� ���������
		SF - ��������� �� ��������� SIGN_FILE_RESULT::SIGNING_FILE, ������� ��������� ������������� ����(����������� �������� � ���������� ���� ���������
		pfSignerSign - ��������� �� ������� SignerSign
	���������, ��� ������ ������� ������� ��������� �������������, ����:
		1)pfSignerSign �� ����� nullptr � ��������� �� �������������� ������ �������������� �������
		2)SF->SignCertificate �� ����� nullptr � ��������� �� �������������� ������ �����������
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
		signerProviderInfo.pwszPvkFileName = (LPWSTR)L"C:\\Users\\blbul\\OneDrive\\���������\\�����������\\CyberForum.pvk";*/
		HRESULT hSignerSignResult = pfSignerSign(&signerSubjectInfo, &signerCert, &signerSignatureInfo, NULL, NULL, NULL, NULL);
		if (SUCCEEDED(hSignerSignResult))result.FileIsSigned = true;
		else {
			//� ������ ���� ��������� ��������� ����
			WSTRING ErrorText = L"�� ������� ��������� ���� \"";
			ErrorText += SF->SigningFileName;
			ErrorText += L"\" �������� ��������!";
			result.ErrorInfo.ErrorString = WWS::ErrorString(ErrorText.c_str(), hSignerSignResult);
			result.ErrorInfo.ErrorCaption = L"������ ������� �����";
			result.ErrorInfo.ErrorCode = hSignerSignResult;
		}
	}
	else {
		result.ErrorInfo = WWS::ErrorString(L"�������� ������ ������� SignerSignEx, �� �� ����� ��������� nullptr!");
		result.ErrorInfo.ErrorCaption = L"�������� ������ �������!";
		result.InitializationFailed = true;
	}
	return result;
}
//������� ��������� ����������
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
								WSTRING ErrorText = L"�� ������� ��������� ���������� \"" + (*CertificateHref) + L"\"!";
								LCR.ErrorInfo = WWS::ErrorString(ErrorText.c_str());
								LCR.ErrorInfo.ErrorCaption = L"������ �������� �����������!";
							}
							//CertCloseStore(hCertStore, CERT_CLOSE_STORE_FORCE_FLAG);
						}
						else {
							WSTRING ErrorText = L"�� ������� ��������� ���������� \"" + (*CertificateHref) + L"\"!";
							LCR.ErrorInfo = WWS::ErrorString(ErrorText.c_str());
							LCR.ErrorInfo.ErrorCaption = L"������ �������� �����������!";
						}
						UnmapViewOfFile(CertificateBlob.pbData);
					}
					else {
						WSTRING ErrorText = L"�� ������� �������� ��������� �� ������� � ������ � ������������ ����� \"" + (*CertificateHref) + L"\"!";
						LCR.ErrorInfo = WWS::ErrorString(ErrorText.c_str());
						LCR.ErrorInfo.ErrorCaption = L"������ ��������� ���������!";
					}
					CloseHandle(hCertMapping);
				}
				else {
					WSTRING ErrorText = L"�� ���������� ���� ����������� \"" + (*CertificateHref) + L"\" � ������";
					LCR.ErrorInfo = WWS::ErrorString(ErrorText.c_str());
					LCR.ErrorInfo.ErrorCaption = L"������ ����������� ����� � ������!";
				}
			}
			else {
				WSTRING ErrorText = L"�� �������� ������ ����� ����������� \"" + (*CertificateHref) + L"\"";
				LCR.ErrorInfo = WWS::ErrorString(ErrorText.c_str());
				LCR.ErrorInfo.ErrorCaption = L"������ ��������� ������� ����� �����������";
			}
			CloseHandle(hCertFile);
		}
		else {
			WSTRING ErrorText = L"�� ������� ������� ���� ����������� \"" + (*CertificateHref) + L"\"";
			LCR.ErrorInfo = WWS::ErrorString(ErrorText.c_str());
			LCR.ErrorInfo.ErrorCaption = L"������ �������� �����!";
		}
		//if (pCertContext)CertFreeCertificateContext(pCertContext);


	}
	return LCR;
}

