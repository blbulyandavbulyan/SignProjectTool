// WWS.cpp : Определяет экспортированные функции для приложения DLL.
//Данная библиотека предназначена для работы со строками
// Здесь будут функции для UNICODE и ASCII
#include "stdafx.h"
#include "WWS.h"
/****************************
******Unicode Version********
****************************/
/***********************************************************************
*Данные функции используются при обработке аргументов командной строки**
***********************************************************************/

WWSDLL_API WSTRINGARRAY WWS::BreakAStringIntoAnArrayOfStringsByCharacter(WSTRING StringToBreak, WCHAR CharacterToBreak) {// данная функция разбивает входную строку StringToBreak на массив строк по заданному символу CharacterToBreak
	WSTRING StringForProcessing = WWS::DeleteTwoCharactersInARow(StringToBreak, CharacterToBreak);
	WSTRINGARRAY TSA;
	if (StringToBreak == L"") {
		return TSA;
	}
	if (StringForProcessing[0] == CharacterToBreak) {
		StringForProcessing.erase(StringForProcessing.begin());
	}
	if (StringForProcessing[StringForProcessing.size() - 1] == CharacterToBreak) {
		StringForProcessing.erase(StringForProcessing.end());
	}
	WSTRING ts;
	size_t StringForProcessingSize = StringForProcessing.size(), StringForProcessingSizeSmallerByOne = StringForProcessingSize - 1;
	for (size_t i = 0; i < StringForProcessingSize; i++) {
		if ((StringForProcessing[i] == CharacterToBreak)) {
			TSA.push_back(ts);
			ts.clear();
		}
		else if (i == StringForProcessingSizeSmallerByOne) {
			ts += StringForProcessing[i];
			TSA.push_back(ts);
			ts.clear();
		}
		else {
			ts += StringForProcessing[i];
		}
	}
	return TSA;
}
WWSDLL_API WSTRING WWS::DeleteTwoCharactersInARow(WSTRING StringForDeleteTwoCharactersInARow, WCHAR SymbolForDelete) {// данная функция удаляет заданный дублирующийся символ
	WSTRING ResultString;
	size_t StringSize = StringForDeleteTwoCharactersInARow.size();
	ResultString += StringForDeleteTwoCharactersInARow[0];
	size_t Counter = 0;
	for (size_t i = 1; i < StringForDeleteTwoCharactersInARow.size(); i++) {
		if (ResultString[Counter] == StringForDeleteTwoCharactersInARow[i] == SymbolForDelete)continue;
		else {
			ResultString += StringForDeleteTwoCharactersInARow[i];
			Counter++;
		}
	}
	return ResultString;
}
/***********************************************************************
*Данные функции выводят диалоговое окно с текстом ошибки****************
***********************************************************************/
WWSDLL_API int WWS::MessageError(LPCWSTR ErrorText, LPCWSTR ErrorCaption, HWND hWnd) {//данная функция сама получает код ошибки с помощью GetLastError
	int result = 0;
	DWORD RHKError = GetLastError();
	LPWSTR BufferForFormatMessage = nullptr;
	DWORD FMResult = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, RHKError, LANG_USER_DEFAULT, (LPWSTR)&BufferForFormatMessage, NULL, nullptr);
	if (FMResult == 0) {
		WSTRING FormatedErrorMessage = ErrorText;
		FormatedErrorMessage += L"\nПричина ошибки: Не удалось узнать причину возникновения ошибки!";
		result = MessageBoxW(hWnd, FormatedErrorMessage.c_str(), ErrorCaption, MB_OK | MB_ICONERROR);
	}
	else {
		WSTRING FormatedErrorMessage = ErrorText;
		FormatedErrorMessage += L"\nПричина ошибки: ";
		FormatedErrorMessage += BufferForFormatMessage;
		result = MessageBoxW(hWnd, FormatedErrorMessage.c_str(), ErrorCaption, MB_OK | MB_ICONERROR);
	}
	if (HeapFree(GetProcessHeap(), NULL, BufferForFormatMessage) == NULL) {
		WSTRING HeapErrorMessage = L"Не удалось освободить буфуер при обработке предыдущей ошибки!\n";
		HeapErrorMessage += (L"Код ошибки: " + std::to_wstring(GetLastError()));
		result = MessageBoxW(hWnd, HeapErrorMessage.c_str(), L"Ошибка освобождения буфера!", MB_OK | MB_ICONERROR);
	}
	return result;
}
//данная функция требует код ошибки HRESULT
WWSDLL_API int WWS::MessageError(LPCWSTR ErrorText, LPCWSTR ErrorCaption, HWND hWnd, HRESULT hErrorCode) {
	int result = 0;
	LPWSTR BufferForFormatMessage = nullptr;
	DWORD FMResult = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, hErrorCode, LANG_USER_DEFAULT, (LPWSTR)&BufferForFormatMessage, NULL, nullptr);
	if (FMResult == 0) {
		WSTRING FormatedErrorMessage = ErrorText;
		FormatedErrorMessage += L"\nПричина ошибки: Не удалось узнать причину возникновения ошибки!";
		result = MessageBoxW(hWnd, FormatedErrorMessage.c_str(), ErrorCaption, MB_OK | MB_ICONERROR);
	}
	else {
		WSTRING FormatedErrorMessage = ErrorText;
		FormatedErrorMessage += L"\nПричина ошибки: ";
		FormatedErrorMessage += BufferForFormatMessage;
		result = MessageBoxW(hWnd, FormatedErrorMessage.c_str(), ErrorCaption, MB_OK | MB_ICONERROR);
	}
	if (HeapFree(GetProcessHeap(), NULL, BufferForFormatMessage) == NULL) {
		WSTRING HeapErrorMessage = L"Не удалось освободить буфуер при обработке предыдущей ошибки!\n";
		HeapErrorMessage += (L"Код ошибки: " + std::to_wstring(GetLastError()));
		result = MessageBoxW(hWnd, HeapErrorMessage.c_str(), L"Ошибка освобождения буфера!", MB_OK | MB_ICONERROR);
	}
	return result;
}
/****************************************************************
*данные функции выводят сообщение об ошибке в консоль отладчика**
*****************************************************************/
WWSDLL_API void WWS::MessageDebug(LPCWSTR ErrorText, LPCWSTR ErrorCaption) {//данная функция сама получает код ошибки с помощью GetLastError
	DWORD RHKError = GetLastError();
	LPWSTR BufferForFormatMessage = nullptr;
	DWORD FMResult = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, RHKError, LANG_USER_DEFAULT, (LPSTR)&BufferForFormatMessage, NULL, nullptr);
	if (FMResult == 0) {
		WSTRING FormatedErrorMessage = ErrorCaption;
		FormatedErrorMessage += L": ";
		FormatedErrorMessage += ErrorText;
		FormatedErrorMessage += L"\nПричина ошибки: Не удалось узнать причину возникновения ошибки!\n";
		OutputDebugStringW(FormatedErrorMessage.c_str());
	}
	else {
		WSTRING FormatedErrorMessage = ErrorCaption;
		FormatedErrorMessage += L": ";
		FormatedErrorMessage += ErrorText;
		FormatedErrorMessage += L"\nПричина ошибки: ";
		FormatedErrorMessage += BufferForFormatMessage;
		FormatedErrorMessage += L"\n";
		OutputDebugStringW(FormatedErrorMessage.c_str());
	}
	if (HeapFree(GetProcessHeap(), NULL, BufferForFormatMessage) == NULL) {
		WSTRING HeapErrorMessage = L"Не удалось освободить буфуер при обработке предыдущей ошибки!\n";
		HeapErrorMessage += (L"Код ошибки: " + std::to_wstring(GetLastError()) + L"\n");
		OutputDebugStringW(HeapErrorMessage.c_str());
	}
}
//данная функция требует код ошибки HRESULT
WWSDLL_API void WWS::MessageDebug(LPCWSTR ErrorText, LPCWSTR ErrorCaption, HRESULT hErrorCode) {
	LPWSTR BufferForFormatMessage = nullptr;
	DWORD FMResult = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, hErrorCode, LANG_USER_DEFAULT, (LPSTR)&BufferForFormatMessage, NULL, nullptr);
	if (FMResult == 0) {
		WSTRING FormatedErrorMessage = ErrorCaption;
		FormatedErrorMessage += L": ";
		FormatedErrorMessage += ErrorText;
		FormatedErrorMessage += L"\nПричина ошибки: Не удалось узнать причину возникновения ошибки!\n";
		OutputDebugStringW(FormatedErrorMessage.c_str());
	}
	else {
		WSTRING FormatedErrorMessage = ErrorCaption;
		FormatedErrorMessage += L": ";
		FormatedErrorMessage += ErrorText;
		FormatedErrorMessage += L"\nПричина ошибки: ";
		FormatedErrorMessage += BufferForFormatMessage;
		FormatedErrorMessage += L"\n";
		OutputDebugStringW(FormatedErrorMessage.c_str());
	}
	if (HeapFree(GetProcessHeap(), NULL, BufferForFormatMessage) == NULL) {
		WSTRING HeapErrorMessage = L"Не удалось освободить буфуер при обработке предыдущей ошибки!\n";
		HeapErrorMessage += (L"Код ошибки: " + std::to_wstring(GetLastError()) + L"\n");
		OutputDebugStringW(HeapErrorMessage.c_str());
	}
}

WWSDLL_API WSTRING WWS::ErrorString(LPCWSTR ErrorText, HRESULT hErrorCode) {
	WSTRING result;
	LPWSTR BufferForFormatMessage = nullptr;
	DWORD FMResult = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, hErrorCode, LANG_USER_DEFAULT, (LPWSTR)&BufferForFormatMessage, NULL, nullptr);
	if (FMResult == 0) {
		result = ErrorText;
		result += (L"\nКод Ошибки: " + std::to_wstring(hErrorCode));
		result += L"\nПричина ошибки: Не удалось узнать причину возникновения ошибки!";
	}
	else {
		result = ErrorText;
		result += (L"\nКод Ошибки: " + std::to_wstring(hErrorCode));
		result += L"\nПричина ошибки: ";
		result += BufferForFormatMessage;

	}
	if (HeapFree(GetProcessHeap(), NULL, BufferForFormatMessage) == NULL) {
		result += L"\nНе удалось освободить буфуер при обработке предыдущей ошибки!\n";
		result += (L"Код ошибки: " + std::to_wstring(GetLastError()));
	}
	return result;
}

WWSDLL_API WWS::WWS_ERROR_REPORT_STRUCTW WWS::ErrorString(LPCWSTR ErrorText) {
	WWS::WWS_ERROR_REPORT_STRUCTW result;
	result.ErrorCode = GetLastError();
	LPWSTR BufferForFormatMessage = nullptr;
	DWORD FMResult = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, result.ErrorCode, LANG_USER_DEFAULT, (LPWSTR)&BufferForFormatMessage, NULL, nullptr);
	if (FMResult == 0) {
		result.ErrorString = ErrorText;
		result.ErrorString += (L"\nКод Ошибки: " + std::to_wstring(result.ErrorCode));
		result.ErrorString += L"\nПричина ошибки: Не удалось узнать причину возникновения ошибки!";
	}
	else {
		result.ErrorString = ErrorText;
		result.ErrorString += (L"\nКод Ошибки: " + std::to_wstring(result.ErrorCode));
		result.ErrorString += L"\nПричина ошибки: ";
		result.ErrorString += BufferForFormatMessage;
	}
	if (HeapFree(GetProcessHeap(), NULL, BufferForFormatMessage) == NULL) {
		result.ErrorString += L"Не удалось освободить буфуер при обработке предыдущей ошибки!\n";
		result.ErrorString += (L"Код ошибки: " + std::to_wstring(GetLastError()));
	}
	return result;
}