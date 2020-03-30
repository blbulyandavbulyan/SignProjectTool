#pragma once
//#include "stdafx.h"
WSTRINGARRAY OpenFiles(HWND hWnd, COMDLG_FILTERSPEC* cmf, UINT cmfSize, LPCWSTR TitleFileDialog, LPCWSTR OKButtonTitle, bool Multiselect);//функция предназначена для показа диалогового окна выбора файлов/файла, возвращает массив строк с полными путями к файлам/возвраает массив строк, содержащий один путь к выбранному файлу, соответсвенно
bool FileReadebleExists(LPCTSTR fname);//функция проверяет существует ли файл и доступен ли он для чтения