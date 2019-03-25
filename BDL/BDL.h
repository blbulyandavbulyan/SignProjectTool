#pragma once
/*******************************************Blbulyan Dialog Library********************************************************
*данная бибилотека содержит основные диалоги и функции обработки их сообщений, используемые в программах Blbulyan Software*
**************************************************************************************************************************/
#include "stdafx.h"
#include "resource.h"
#ifdef BDL_EXPORTS
#define BDLDLL_API __declspec(dllexport)
//#pragma comment(linker, "/EXPORT:EnterPasswordDlgProcA@1=EnterPasswordDlgProcA")
//#pragma comment(linker, "/EXPORT:EnterPasswordDlgProcW@2=EnterPasswordDlgProcW")
#else
#define BDLDLL_API __declspec(dllimport)
#endif
#ifdef BDL_ENABLE_PROTOTYPES
namespace BDL {
	BDLDLL_API INT_PTR CALLBACK EnterPasswordDlgProcA(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	BDLDLL_API INT_PTR CALLBACK EnterPasswordDlgProcW(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	#ifdef BDL_EXPORTS
	HWND CreateToolTipA(HWND hDlgItem, HWND hDlg, LPSTR pszText);
	HWND CreateToolTipW(HWND hDlgItem, HWND hDlg, PWSTR pszText);
	WSTRING strtowstr(const STRING &str);
	#endif
}
#endif
namespace BDL {
	typedef struct EnterPasswordDlgInitParamA {//структура инициализационных параметров для функции EnterPasswordDlgProcA
		CHARVECTOR *Password;//указатель на объект, в котором сохранится введённый пароль
		STRING Caption;//заголовок диалогового окна ввода пароля 
		STRING EditPasswordCaption;//подсказка, отображаемая в поле ввода пароля
		STRING OkButtonCaption;//текст, который отображается в кнопке IDОК выводимого диалога
		STRING CancelButtonCaption;//текст, который отображается в кнопке IDCANCEL выводимого диалог
		STRING ToolTipCaption[2];//массив со строками, которые будут использоваться в подсказсках, значение по умолчанию будет использоваться в том случае, если соотвествующая строка - пустая
		//Описание данного массива:
		// ToolTipCaption[0] - текст подсказски, который отображается над кнопкой "Показать/скрыть пароль", когда её значение "показать пароль", по умолчанию значение этой подсказски "Показать пароль"
		//ToolTipCaption[1] - текст подсказски, который отображается над кнопкой "Показать/скрыть пароль", когда её значение "скрыть пароль", по умолчанию значение этой подсказски "Скрыть пароль"
		CHAR PasswordChar = '\0';//символ, отображаемый при вводе пароля, если хоитие чтобы использовался символ по умолчанию используйте значение '\0'
		HICON hIconCaption = nullptr;// дескриптор иконки отображаемой в заголовке диалогового окна
		bool HasToolTip = false;//данная переменная показывает будет ли диалог иметь всплывающие подсказски
	} *PEnterPasswordDlgInitParamA;
	typedef struct EnterPasswordDlgInitParamW {//структура инициализационных параметров для функции EnterPasswordDlgProcW
		WCHARVECTOR *Password;//указатель на объект, в котором сохранится введённый пароль
		WSTRING Caption;//заголовок диалогового окна ввода пароля 
		WSTRING EditPasswordCaption;//подсказка, отображаемая в поле ввода пароля
		WSTRING OkButtonCaption;//текст, который отображается в кнопке IDОК выводимого диалога
		WSTRING CancelButtonCaption;//текст, который отображается в кнопке IDCANCEL выводимого диалог
		WSTRING ToolTipCaption[2];//массив со строками, которые будут использоваться в подсказсках, значение по умолчанию будет использоваться в том случае, если соотвествующая строка - пустая
		//Описание данного массива:
		// ToolTipCaption[0] - текст подсказски, который отображается над кнопкой IDC_ENTERPASSWORDDLG_SHOW_PASSWORD, когда её значение "показать пароль", по умолчанию значение этой подсказски "Показать пароль"
		//ToolTipCaption[1] - текст подсказски, который отображается над кнопкой IDC_ENTERPASSWORDDLG_SHOW_PASSWORD, когда её значение "скрыть пароль", по умолчанию значение этой подсказски "Скрыть пароль"
		WCHAR PasswordChar = '\0';//символ, отображаемый при вводе пароля, если хоитие чтобы использовался символ по умолчанию используйте значение '\0'
		HICON hIconCaption = nullptr;// дескриптор иконки отображаемой в заголовке диалогового окна
		bool HasToolTip = false;//данная переменная показывает будет ли диалог иметь всплывающие подсказски
	} *PEnterPasswordDlgInitParamW;
}