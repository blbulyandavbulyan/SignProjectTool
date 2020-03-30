#include "stdafx.h"
//функция предназначена для показа диалогового окна выбора файлов/файла, возвращает массив строк с полными путями к файлам/возвраает массив строк, содержащий один путь к выбранному файлу, соответсвенно
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
				if (hr != S_OK)WWS::MessageError(L"Не удалось установить заголовок для диалогового окна открытия файлов!", L"Ошибка установки заголовка!", hWnd, hr);
				hr = pFileOpen->SetOkButtonLabel(OKButtonTitle);
				if (hr != S_OK)WWS::MessageError(L"Не удалось установить текст кнопки OK для диалогового окна открытия файлов!", L"Ошибка установки текста кнопки OK!", hWnd, hr);
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
										else WWS::MessageError(L"Не удалось получить имя открытого вами файла, файл не будет добавлен!", L"Ошибка получения имени открытого файла!", hWnd, hr);
										MyFile->Release();
									}
									else WWS::MessageError(L"Не удалось получить один из открытых вами файлов, файл не будет добавлен!", L"Ошибка получения открытого файла!", hWnd, hr);
								}
							}
							else WWS::MessageError(L"Не удалось получить количество открытых вами файлов!", L"Ошибка получения кол-ва открытых файлов!", hWnd, hr);
							pItem->Release();
						}
						else WWS::MessageError(L"Не удалось получить открытые вами файлы!", L"Ошибка получения открытых вами файлов!", hWnd, hr);
					}
					else if (hr != HRESULT_FROM_WIN32(ERROR_CANCELLED)) WWS::MessageError(L"Не удалось показать диалоговое окно открытия файлов, дальнейшее открытие файлов невозможно!", L"Ошибка показа диалогового окна!", hWnd, hr);
					pFileOpen->Release();
				}
				else WWS::MessageError(L"Не удалось установить необходимые опции для файлового диалога, дальнейшее открытие файлов невозможно!", L"Ощибка установки опций!", hWnd, hr);
			}
			else WWS::MessageError(L"Не удалось установить фильтры допустимых файлов, невозможно добавить файлы!", L"Ошибка установки фильтров для файлового диалога!", hWnd, hr);
		}
		else WWS::MessageError(L"Не удалось создать объект CLSID_FileOpenDialog, дальнейшее открытие файлов невозможно!", L"Ошибка создания объекта CLSID_FileOpenDialog!", hWnd, hr);
		CoUninitialize();
	}
	else WWS::MessageError(L"Не удалось инициализировать библиотеку COM!", L"Ошибка инициализиции библиотеки COM!", hWnd, hr);
	return FilesNames;
}
//функция проверяет существует ли файл и доступен ли он для чтения
bool FileReadebleExists(LPCTSTR fname){
	HANDLE hFile = CreateFileW(fname, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (INVALID_HANDLE_VALUE != hFile) {
		CloseHandle(hFile);
		return true;
	}
	return false;
}