#pragma once
//#include "stdafx.h"
WSTRINGARRAY OpenFiles(HWND hWnd, COMDLG_FILTERSPEC* cmf, UINT cmfSize, LPCWSTR TitleFileDialog, LPCWSTR OKButtonTitle, bool Multiselect);//������� ������������� ��� ������ ����������� ���� ������ ������/�����, ���������� ������ ����� � ������� ������ � ������/��������� ������ �����, ���������� ���� ���� � ���������� �����, �������������
bool FileReadebleExists(LPCTSTR fname);//������� ��������� ���������� �� ���� � �������� �� �� ��� ������