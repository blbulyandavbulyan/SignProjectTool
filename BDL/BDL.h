#pragma once
/*******************************************Blbulyan Dialog Library********************************************************
*данная бибилотека содержит основные диалоги и функции обработки их сообщений, используемые в программах Blbulyan Software*
**************************************************************************************************************************/
#include "stdafx.h"
#include "resource.h"
#ifdef BDL_EXPORTS
#define BDLDLL_API __declspec(dllexport)
#else
#define BDLDLL_API __declspec(dllimport)
#endif
#ifdef BDL_ENABLE_PROTOTYPES
namespace BDL {
	/*Unicode DlgProc`s versions*/
	BDLDLL_API INT_PTR CALLBACK EnterPasswordDlgProcW(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);//функция обработки сообщений для диалога IDDBDL_ENTERPASSWORD
	#ifdef BDL_EXPORTS
	/*Unicode версии вспомогательных функций*/
	HWND CreateToolTipW(HWND hDlgItem, HWND hDlg, LPCWSTR pszText, HINSTANCE hInst);//функция создаёт всплывающую подсказску
	bool CompareTwoLines(LPCWSTR str1, LPCWSTR str2);
	#endif
}
#endif
namespace BDL {
	typedef struct EnterPasswordDlgInitParamW {//структура инициализационных параметров для функции EnterPasswordDlgProcW
		LPWSTR *Password = nullptr;//указатель на объект, в котором сохранится введённый пароль
		LPCWSTR Caption = nullptr;//заголовок диалогового окна ввода пароля 
		LPCWSTR EditPasswordCaption = nullptr;//подсказка, отображаемая в поле ввода пароля
		LPCWSTR OkButtonCaption = nullptr;//текст, который отображается в кнопке IDОК выводимого диалога
		LPCWSTR CancelButtonCaption = nullptr;//текст, который отображается в кнопке IDCANCEL выводимого диалог
		LPCWSTR ClearButtonCaption = nullptr;//Текст отображаемый в кнопке IDC_DBL_ENTERPASSWORDCLEAR
		LPCWSTR ToolTipCaption[2] = {
			nullptr,
			nullptr
		};//массив со строками, которые будут использоваться в подсказсках, значение по умолчанию будет использоваться в том случае, если соотвествующая строка - пустая
		//Описание данного массива:
		// ToolTipCaption[0] - текст подсказски, который отображается над кнопкой IDC_ENTERPASSWORDDLG_SHOW_PASSWORD, когда её значение "показать пароль", по умолчанию значение этой подсказски "Показать пароль"
		//ToolTipCaption[1] - текст подсказски, который отображается над кнопкой IDC_ENTERPASSWORDDLG_SHOW_PASSWORD, когда её значение "скрыть пароль", по умолчанию значение этой подсказски "Скрыть пароль"
		WCHAR PasswordChar = '\0';//символ, отображаемый при вводе пароля, если хоитие чтобы использовался символ по умолчанию используйте значение '\0'
		HICON hIconCaption = nullptr;// дескриптор иконки отображаемой в заголовке диалогового окна
		BOOL HasToolTip = FALSE;//данная переменная показывает будет ли диалог иметь всплывающие подсказски
	} *PEnterPasswordDlgInitParamW;
}