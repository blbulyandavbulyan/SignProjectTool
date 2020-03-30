#pragma once
#include "resource.h"
//структура с основными параметрами программы
#define DEFUAULT_SIGN_HASH_ALGORITHM CALG_SHA_256//макрос определяющий алгоритм хэширования по умолчанию при подписи
#define ROOT_REGISTRY_KEY_SIGNPROJECT_TOOL L"Software\\Blbulyan Software\\SignProjectTool"//Макрос, определяющий корневую ветку в реестре для данной программы, он будет использоваться относительно HKEY_CURRENT_USER
struct ProgrammSettings {
	void SaveProgrammSettingsInRegistry(unsigned sfSave);//данная функция преназначена для сохранения параметров программы в реестр Windows
	void LoadProgrammSettingsOfRegistry(unsigned sfLoad);//данная функция предназначена для загрузки параметров программы из реестра Windows
	enum REGISTRY_QUERY { SIGN_CERTIFICATE_FILE_NAME = 0b00000001, ALG_HASH = 0b00000010 };//перечисление, оперделяющее флаги, комбинация которых передаётся вышеописанным функциям, они позволяют задать какие параметры следует (загружать из реестра)/(сохранять в реестре)
	//флаги комбинировать с помощью операции побитового ИЛИ, пример вызова LoadProgrammSettingsOfRegistry с заданными параметрами:
	//LoadProgrammSettingsOfRegistry(SIGN_CERTIFICATE_FILE_NAME | ALG_HASH) - функция загрузит два параметра из реестра, путь к сертификату и алгоритм хеша
	//LoadProgrammSettingsOfRegistry(SIGN_CERTIFICATE_FILE_NAME) - функция загрузит только полный путь к сертификату
	//LoadProgrammSettingsOfRegistry(ALG_HASH) - функция загрузит только алгоритм хеширования
	//соотвственно флаг SIGN_CERTIFICATE_FILE_NAME - предназначен для загрузки полного пути к сертификату для подписи из реестра Windows,
	//а флаг ALG_HASH - предназначен для загрузки алгоритма хеширования из реестра Windows
	enum COMMAND_AFTER_SIGNING_FILES {
		CASF_DELETE_SUCCESSFULLY_SIGNED_FILES_FROM_LIST = 0b0000000000000001,
		CASF_DELETE_NOT_SUCCESSFULLY_SIGNED_FILES_FROM_LIST = 0b0000000000000010,
		CASF_DELETE_ALL_FILES_FROM_LIST = 0b0000000000000011
	};
	/*
		Команды, которые могут быть выполнены над списком:
			0bxxxxxx00 - ничего не удалять из списка
			0bxxxxxxx1 - удалить успешно подписанные файлы из списка
			0b00000010 - удалить недаучноподписанные файлы из списка
			0bxxxxxx11 - удалить все файлы из списка

	*/
	WSTRINGARRAY FilesForCertification;//список файлов для подписи
	WSTRING CertificateFile;// имя файла сертификата для подписи им
	WSTRINGARRAY HttpTimeStampServers;//список TimeStamp серверов
	static const WSTRINGARRAY CommandLineValidArguments;//массив с допустимыми аргументами командной строки
	bool CertificateInCertStore = false;//хранится ли сертификат, которым будут подписываться файлы в хранилище сертификатов (задаётся при начальной настройке программы
	ALG_ID HashAlgorithmId = DEFUAULT_SIGN_HASH_ALGORITHM;//алгоритм хеширования при подписи поумолчанию
	bool LoadSettingsFromRegistry = true;//загружать настройки из реестра Windows
	bool SaveSettingsInRegistry = true;//сохранять настройки в реестр Windows
	WORD CommandToExecuteAfterSigningFiles = 0;//команда для исполнения после подписи файлов
};
//структура описывающая элементы рантайма, состояние программы в рантайме, данные в ней могут менятся в зависимости от запуска к запуску
struct ProgrammStatement {
	HINSTANCE hInst = NULL;//экземпляр приложения
	WNDPROC DefaultConsoleWindowProc = NULL;//процедура консоли по умолчанию(возможно не используется)
	HANDLE hSignFilesThread = nullptr;//дескриптор этого потока
	BOOL ConsoleIsAlloced = FALSE;//выделена ли консоль
	HWND hRootWnd = NULL;//дескриптор главного окна
};
