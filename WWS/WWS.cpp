// WWS.cpp : Определяет экспортированные функции для приложения DLL.
//Данная библиотека предназначена для работы со строками
// Здесь будут функции для UNICODE и ASCII
#include "stdafx.h"
#include "WWS.h"
// Unicode Version
MYDLLLIBRARY_API WSTRINGARRAY BreakAStringIntoAnArrayOfStringsByCharacter(WSTRING StringToBreak, WCHAR CharacterToBreak) {// данная функция разбивает входную строку StringToBreak на массив строк по заданному символу CharacterToBreak
	WSTRING StringForProcessing = DeleteTwoCharactersInARow(StringToBreak, CharacterToBreak);
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
MYDLLLIBRARY_API WSTRING DeleteTwoCharactersInARow(WSTRING StringForDeleteTwoCharactersInARow, WCHAR SymbolForDelete) {// данная функция удаляет заданный дублирующийся символ
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
// функция выводит диалоговое окно с ошибкой (функция получает код ошибки с помощью GetLastError)
MYDLLLIBRARY_API void MessageError(WSTRING ErrorText, WSTRING ErrorCaption, HWND hWnd) {
	DWORD RHKError = GetLastError();
	LPWSTR BufferForFormatMessage = nullptr;
	DWORD FMResult = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, RHKError, LANG_USER_DEFAULT, (LPWSTR)&BufferForFormatMessage, NULL, nullptr);
	if (FMResult == 0) {
		MessageBoxW(hWnd, ErrorText.c_str(), ErrorCaption.c_str(), MB_OK | MB_ICONERROR);
		MessageError(L"Не удалось узнать причину возникновения предыдущей ошибки!", L"Ошибка распознавания предыдущей ошибки!", hWnd);
	}
	else {
		WSTRING FormatedErrorMessage = ErrorText;
		FormatedErrorMessage += L"\nПричина ошибки: ";
		FormatedErrorMessage += BufferForFormatMessage;
		MessageBoxW(hWnd, FormatedErrorMessage.c_str(), ErrorCaption.c_str(), MB_OK | MB_ICONERROR);
	}
	if (HeapFree(GetProcessHeap(), NULL, BufferForFormatMessage) == NULL) {
		WSTRING HeapErrorMessage = L"Не удалось освободить буфуер при обработке предыдущей ошибки!\n";
		HeapErrorMessage += (L"Код ошибки: " + std::to_wstring(GetLastError()));
		MessageBoxW(hWnd, HeapErrorMessage.c_str(), L"Ошибка освобождения буфера!", MB_OK | MB_ICONERROR);
	}
}
//функция выводит диалоговое окно с ошибкой по заданному hErrorCode
MYDLLLIBRARY_API void MessageError(WSTRING ErrorText, WSTRING ErrorCaption, HWND hWnd, HRESULT hErrorCode) {
	LPWSTR BufferForFormatMessage = nullptr;
	DWORD FMResult = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, hErrorCode, LANG_USER_DEFAULT, (LPWSTR)&BufferForFormatMessage, NULL, nullptr);
	if (FMResult == 0) {
		MessageBoxW(hWnd, ErrorText.c_str(), ErrorCaption.c_str(), MB_OK | MB_ICONERROR);
		MessageError(L"Не удалось узнать причину возникновения предыдущей ошибки!", L"Ошибка распознавания предыдущей ошибки!", hWnd);
	}
	else {
		WSTRING FormatedErrorMessage = ErrorText;
		FormatedErrorMessage += L"\nПричина ошибки: ";
		FormatedErrorMessage += BufferForFormatMessage;
		MessageBoxW(hWnd, FormatedErrorMessage.c_str(), ErrorCaption.c_str(), MB_OK | MB_ICONERROR);
	}
	if (HeapFree(GetProcessHeap(), NULL, BufferForFormatMessage) == NULL) {
		WSTRING HeapErrorMessage = L"Не удалось освободить буфуер при обработке предыдущей ошибки!\n";
		HeapErrorMessage += (L"Код ошибки: " + std::to_wstring(GetLastError()));
		MessageBoxW(hWnd, HeapErrorMessage.c_str(), L"Ошибка освобождения буфера!", MB_OK | MB_ICONERROR);
	}
}
// ASCII version
MYDLLLIBRARY_API ASTRINGARRAY BreakAStringIntoAnArrayOfStringsByCharacter(ASTRING StringToBreak, CHAR CharacterToBreak) {// данная функция разбивает входную строку StringToBreak на массив строк по заданному символу CharacterToBreak
	ASTRING StringForProcessing = DeleteTwoCharactersInARow(StringToBreak, CharacterToBreak);
	ASTRINGARRAY TSA;
	if (StringToBreak == "") {
		return TSA;
	}
	if (StringForProcessing[0] == CharacterToBreak) {
		StringForProcessing.erase(StringForProcessing.begin());
	}
	if (StringForProcessing[StringForProcessing.size() - 1] == CharacterToBreak) {
		StringForProcessing.erase(StringForProcessing.end());
	}
	ASTRING ts;
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
MYDLLLIBRARY_API ASTRING DeleteTwoCharactersInARow(ASTRING StringForDeleteTwoCharactersInARow, CHAR SymbolForDelete) {// данная функция удаляет заданный дублирующийся символ
	ASTRING ResultString;
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
// функция выводит диалоговое окно с ошибкой (функция получает код ошибки с помощью GetLastError)
MYDLLLIBRARY_API void MessageError(ASTRING ErrorText, ASTRING ErrorCaption, HWND hWnd) {
	DWORD RHKError = GetLastError();
	LPSTR BufferForFormatMessage = nullptr;
	DWORD FMResult = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, RHKError, LANG_USER_DEFAULT, (LPSTR)&BufferForFormatMessage, NULL, nullptr);
	if (FMResult == 0) {
		MessageBoxA(hWnd, ErrorText.c_str(), ErrorCaption.c_str(), MB_OK | MB_ICONERROR);
		MessageError("Не удалось узнать причину возникновения предыдущей ошибки!", "Ошибка распознавания предыдущей ошибки!", hWnd);
	}
	else {
		ASTRING FormatedErrorMessage = ErrorText;
		FormatedErrorMessage += "\nПричина ошибки: ";
		FormatedErrorMessage += BufferForFormatMessage;
		MessageBoxA(hWnd, FormatedErrorMessage.c_str(), ErrorCaption.c_str(), MB_OK | MB_ICONERROR);
	}
	if (HeapFree(GetProcessHeap(), NULL, BufferForFormatMessage) == NULL) {
		ASTRING HeapErrorMessage = "Не удалось освободить буфуер при обработке предыдущей ошибки!\n";
		HeapErrorMessage += ("Код ошибки: " + std::to_string(GetLastError()));
		MessageBoxA(hWnd, HeapErrorMessage.c_str(), "Ошибка освобождения буфера!", MB_OK | MB_ICONERROR);
	}
}
//функция выводит диалоговое окно с ошибкой по заданному hErrorCode
MYDLLLIBRARY_API void MessageError(ASTRING ErrorText, ASTRING ErrorCaption, HWND hWnd, HRESULT hErrorCode) {
	LPSTR BufferForFormatMessage = nullptr;
	DWORD FMResult = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, hErrorCode, LANG_USER_DEFAULT, (LPSTR)&BufferForFormatMessage, NULL, nullptr);
	if (FMResult == 0) {
		MessageBoxA(hWnd, ErrorText.c_str(), ErrorCaption.c_str(), MB_OK | MB_ICONERROR);
		MessageError("Не удалось узнать причину возникновения предыдущей ошибки!", "Ошибка распознавания предыдущей ошибки!", hWnd);
	}
	else {
		ASTRING FormatedErrorMessage = ErrorText;
		FormatedErrorMessage += "\nПричина ошибки: ";
		FormatedErrorMessage += BufferForFormatMessage;
		MessageBoxA(hWnd, FormatedErrorMessage.c_str(), ErrorCaption.c_str(), MB_OK | MB_ICONERROR);
	}
	if (HeapFree(GetProcessHeap(), NULL, BufferForFormatMessage) == NULL) {
		ASTRING HeapErrorMessage = "Не удалось освободить буфуер при обработке предыдущей ошибки!\n";
		HeapErrorMessage += ("Код ошибки: " + std::to_string(GetLastError()));
		MessageBoxA(hWnd, HeapErrorMessage.c_str(), "Ошибка освобождения буфера!", MB_OK | MB_ICONERROR);
	}
}
