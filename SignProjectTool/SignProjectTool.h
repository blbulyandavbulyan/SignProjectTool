#pragma once

#include "resource.h"/*
class Toolbar{
public:
	Toolbar(HWND hParent, HINSTANCE hInst, );
	~Toolbar(void);

	int     addButton(int id, int bmpRes, unsigned char fsStyle = BTNS_BUTTON, unsigned char fsState = TBSTATE_ENABLED);
	int     addSeparetor(unsigned char fsState = TBSTATE_ENABLED);
	void    OnResize(int w, int h);

private:
	void    CreateToolBar(HWND hParent, HINSTANCE hInst);

	HIMAGELIST  myimg;
	HWND hToolBarWnd = NULL;
	HWND hParent = NULL;
};
Toolbar::Toolbar(HWND hParent)
{
	CreateToolBar(hParent);
}

Toolbar::~Toolbar(void)
{
	ImageList_Destroy(myimg);
}

void Toolbar::CreateToolBar(HWND hParent, HINSTANCE hInst)
{
	const int bitmapSize = 16;

	// Create the toolbar.
	hToolBarWnd = CreateWindowEx(0, TOOLBARCLASSNAME, NULL,
		WS_CHILD | TBSTYLE_WRAPABLE | CCS_NODIVIDER | TBSTYLE_TOOLTIPS, // Styles
		0, 0, 0, 0, hParent, NULL, Window::hModule, NULL);

	if (!handle) {
		MessageBox(0, TEXT("Failed to create a tool bar"), TEXT("NULL"), MB_OK | MB_ICONERROR);
		return;
	}

	SendMessage(handle, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);

	if (!(myimg = ImageList_Create(bitmapSize, bitmapSize, ILC_COLOR24 | ILC_MASK, 1, 16)))
		return;

	SendMessage(handle, TB_SETIMAGELIST, (WPARAM)0, (LPARAM)myimg);

	SendMessage(handle, TB_SETBUTTONSIZE, 0, MAKELPARAM(bitmapSize, bitmapSize));
	ShowWindow(handle, SW_SHOWNORMAL);
}

int Toolbar::addButton(int id, int bmpRes, unsigned char fsStyle, unsigned char fsState)
{
	HBITMAP hBmp = LoadBitmap(Window::hModule, MAKEINTRESOURCE(bmpRes));
	int index = ImageList_AddMasked(myimg, hBmp, RGB(255, 255, 255));
	DeleteObject(hBmp);

	TBBUTTON tbutton = { 0 };
	tbutton.idCommand = id;
	tbutton.iBitmap = index;
	tbutton.fsState = fsState;
	tbutton.fsStyle = fsStyle;

	if (SendMessage(handle, TB_INSERTBUTTON, (WPARAM)0, (LPARAM)& tbutton))
		return index;

	return -1;
}

int Toolbar::addSeparetor(unsigned char fsState)
{
	TBBUTTON tbutton = { 0 };
	tbutton.idCommand = 0;
	tbutton.iBitmap = 0;
	tbutton.fsState = fsState;
	tbutton.fsStyle = BTNS_SEP;

	return (int)SendMessage(handle, TB_INSERTBUTTON, (WPARAM)0, (LPARAM)& tbutton);
}

void Toolbar::OnResize(int w, int h)
{
	SendMessage(handle, TB_AUTOSIZE, 0, 0);
}*/
