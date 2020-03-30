// TestSignPolygon.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include "stdafx.h"
int main()
{
	WSTRING Test, Test2 = L"проверка конвертиацфвфц";
	ASTRING aTest = "Тест конвертации", aTest2;
	UnicodeToAnsi(Test2.c_str(), aTest2);
	AnsiToUnicode(aTest.c_str(), Test);
	if (LoadLibraryA("dawda.dll") == NULL) {
		MessageDebug("Не удалось загрузить DLL", "Ошибка загрузке DLL"); 
	}
    return 0;
}
