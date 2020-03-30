#include "stdafx.h"
//������� ������������� ��� ������ ����������� ���� ������ ������/�����, ���������� ������ ����� � ������� ������ � ������/��������� ������ �����, ���������� ���� ���� � ���������� �����, �������������
WSTRINGARRAY OpenFiles(HWND hWnd, COMDLG_FILTERSPEC* cmf, UINT cmfSize, LPCWSTR TitleFileDialog, LPCWSTR OKButtonTitle, bool Multiselect) {
	WSTRINGARRAY FilesNames;
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
		COINIT_DISABLE_OLE1DDE);
	if (SUCCEEDED(hr)) {
		IFileOpenDialog* pFileOpen;
		// Create the FileOpenDialog object.
		hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
			IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));
		if (SUCCEEDED(hr)) {
			// Show the Open dialog box.
			hr = pFileOpen->SetFileTypes(cmfSize, cmf);
			if (SUCCEEDED(hr)) {
				hr = pFileOpen->SetTitle(TitleFileDialog);
				if (hr != S_OK)WWS::MessageError(L"�� ������� ���������� ��������� ��� ����������� ���� �������� ������!", L"������ ��������� ���������!", hWnd, hr);
				hr = pFileOpen->SetOkButtonLabel(OKButtonTitle);
				if (hr != S_OK)WWS::MessageError(L"�� ������� ���������� ����� ������ OK ��� ����������� ���� �������� ������!", L"������ ��������� ������ ������ OK!", hWnd, hr);
				hr = pFileOpen->SetOptions(FOS_FILEMUSTEXIST | FOS_PATHMUSTEXIST | (Multiselect == TRUE ? FOS_ALLOWMULTISELECT : 0));
				if (SUCCEEDED(hr)) {
					hr = pFileOpen->Show(hWnd);
					// Get the file name from the dialog box.
					if (SUCCEEDED(hr)) {
						IShellItemArray* pItem = nullptr;
						hr = pFileOpen->GetResults(&pItem);
						if (SUCCEEDED(hr)) {
							DWORD FilesCount = 0;
							hr = pItem->GetCount(&FilesCount);
							if (SUCCEEDED(hr)) {
								for (DWORD i = 0; i < FilesCount; i++) {
									IShellItem* MyFile = nullptr;
									hr = pItem->GetItemAt(i, &MyFile);
									if (SUCCEEDED(hr)) {
										PWSTR pszFilePath;
										hr = MyFile->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
										// Display the file name to the user.
										if (SUCCEEDED(hr)) {
											FilesNames.push_back(pszFilePath);
											CoTaskMemFree(pszFilePath);
										}
										else WWS::MessageError(L"�� ������� �������� ��� ��������� ���� �����, ���� �� ����� ��������!", L"������ ��������� ����� ��������� �����!", hWnd, hr);
										MyFile->Release();
									}
									else WWS::MessageError(L"�� ������� �������� ���� �� �������� ���� ������, ���� �� ����� ��������!", L"������ ��������� ��������� �����!", hWnd, hr);
								}
							}
							else WWS::MessageError(L"�� ������� �������� ���������� �������� ���� ������!", L"������ ��������� ���-�� �������� ������!", hWnd, hr);
							pItem->Release();
						}
						else WWS::MessageError(L"�� ������� �������� �������� ���� �����!", L"������ ��������� �������� ���� ������!", hWnd, hr);
					}
					else if (hr != HRESULT_FROM_WIN32(ERROR_CANCELLED)) WWS::MessageError(L"�� ������� �������� ���������� ���� �������� ������, ���������� �������� ������ ����������!", L"������ ������ ����������� ����!", hWnd, hr);
					pFileOpen->Release();
				}
				else WWS::MessageError(L"�� ������� ���������� ����������� ����� ��� ��������� �������, ���������� �������� ������ ����������!", L"������ ��������� �����!", hWnd, hr);
			}
			else WWS::MessageError(L"�� ������� ���������� ������� ���������� ������, ���������� �������� �����!", L"������ ��������� �������� ��� ��������� �������!", hWnd, hr);
		}
		else WWS::MessageError(L"�� ������� ������� ������ CLSID_FileOpenDialog, ���������� �������� ������ ����������!", L"������ �������� ������� CLSID_FileOpenDialog!", hWnd, hr);
		CoUninitialize();
	}
	else WWS::MessageError(L"�� ������� ���������������� ���������� COM!", L"������ ������������� ���������� COM!", hWnd, hr);
	return FilesNames;
}
//������� ��������� ���������� �� ���� � �������� �� �� ��� ������
bool FileReadebleExists(LPCTSTR fname){
	HANDLE hFile = CreateFileW(fname, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (INVALID_HANDLE_VALUE != hFile) {
		CloseHandle(hFile);
		return true;
	}
	return false;
}