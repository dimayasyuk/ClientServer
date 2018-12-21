#include "stdafx.h"
#include <winsock2.h> 
#include <windows.h>
#include <winbase.h>

//Необходимо, чтобы линковка происходила с DLL - библиотекой для работы с сокетом
#pragma warning (disable : 4786)
#pragma comment(lib, "ws2_32.lib")

#include <fstream>
#include <sstream>
#include <vector>
#include <time.h>
#include <direct.h>
#include <signal.h>

#define MY_PORT 5223		  
#define TIMEOUT 1000		  
#define ERROR_WSASTARTUP 1
#define ERROR_SOCKET 2
#define ERROR_BIND 3
#define ERROR_LISTEN 4
#define ACCESS_CODE 0

std::ostringstream B;		  

std::vector<HANDLE> arrHandle;

FILE *mylog;

struct tagINF{
	sockaddr_in* caddr;
	SOCKET* soc;
	HANDLE hID;
};

VOID APIENTRY timer_proc(LPVOID IpArgToCompletionRoutine,
	DWORD dwTimerLowValue,
	DWORD dwTimerHighValue
	);

 
#define PRINTNUSERS if (nclients)\
	printf("%d user on-line\n",nclients);\
else printf("No User on line\n");

CRITICAL_SECTION  critSect;

DWORD WINAPI WorkWithClient(LPVOID info);
void SignalHandler(int signal);

int nclients = 0;
int main(int argc, char* argv[])

{	  
	char buff[1024];		  

	printf("TCP SERVER \n");

	// старт использования библиотеки сокетов процессом
	// (подгружается Ws2_32.dll)
	if (WSAStartup(0x0202, (WSADATA *)&buff[0]))
	{
		return ERROR_WSASTARTUP;
	}
	// Инициализация критической секции и создание сокета
		InitializeCriticalSection(&critSect);
		SOCKET mysocket;
		
		// Если создание сокета завершилось с ошибкой выводим сообщение и
		// выгружаем dll-библиотеку и возвращаем код ошибки
		if ((mysocket = socket(AF_INET, SOCK_STREAM, 0)) < 0){
			WSACleanup();
			return ERROR_SOCKET;
		}

		sockaddr_in local_addr;
		local_addr.sin_family = AF_INET;//AF_INET определяет, что используется сеть для работы с сокетом
		local_addr.sin_port = htons(MY_PORT);//задаем порт
		local_addr.sin_addr.s_addr = 0;

		// Если привязать адрес к сокету не удалось, то выводим сообщение
		// и закрываем открытый сокет.
		// Выгружаем DLL-библиотеку из памяти и возвращаем код ошибки
		if (bind(mysocket, (sockaddr *)&local_addr,
			sizeof(local_addr)))
		{
			closesocket(mysocket);
			WSACleanup();
			return ERROR_BIND;
		}

		// Инициализируем слушающий сокет
		if ((listen(mysocket, SOMAXCONN)) == SOCKET_ERROR)
		{
			closesocket(mysocket);
			WSACleanup();
			return ERROR_LISTEN;
		}

		//клиентский сокет
		SOCKET client_socket;
		sockaddr_in client_addr;

		int client_addr_size = sizeof(client_addr);

		//При успешной установке TCP - соединения,
		//для него создается новый сокет.
		//Функция accept возвращает дескриптор этого сокета.
		//Если произошла ошибка соединения, то возвращается значение INVALID_SOCKET.


		while ((client_socket = accept(mysocket, (sockaddr *)
			&client_addr, &client_addr_size)))
		{
			nclients++;

			//HOSTENT *hst;
			// Пытаемся получить имя хоста
			//hst = gethostbyaddr((char *)
			//	&client_addr.sin_addr.s_addr, 4, AF_INET);


			PRINTNUSERS

			DWORD thID;
			tagINF info;

			info.caddr = &client_addr;
			info.soc = &client_socket; 

			HANDLE hID = CreateThread(NULL, NULL, WorkWithClient,
				&info, NULL, &thID);
			arrHandle.push_back(hID);

		}

		WSACleanup();
		std::vector<HANDLE>::iterator it;
		for (it = arrHandle.begin(); it != arrHandle.end(); it++)
		CloseHandle(*it);
		DeleteCriticalSection(&critSect);
	return 0;
}

void removeSpaces(char *str){
	int count = 0;
	for (int i = 0; str[i]; i++){
		if ((str[i] == ' ') || (str[i] == ':')){
			str[count++] = '_';
		}
		else{
			str[count++] = str[i];
		}
	}
	str[count] = '\0';
	if (str[strlen(str) - 1] == '\n') {
		str[strlen(str) - 1] = '\0';
	}
}

void SignalHandler(int signal) {
	HANDLE hID;
	DuplicateHandle(
		GetCurrentProcess(),
		GetCurrentThread(),
		GetCurrentProcess(),
		&hID,
		0,
		FALSE,
		DUPLICATE_SAME_ACCESS);
	char szCurDir[MAX_PATH];
	getcwd(szCurDir, MAX_PATH);
	std::string file, fullpath = szCurDir;
	fullpath += "\\tmp";
	mkdir(fullpath.c_str());
	fullpath += "\\";
	char data[26];
	time_t result = time(NULL);
	ctime_s(data, sizeof data, &result);
	removeSpaces(data);
	sprintf(szCurDir, "%s.log",data);
	fullpath += szCurDir;
	try {
		std::filebuf file;
		if (file.open(fullpath.c_str(), std::ios::out) == 0) {
			throw (-1);
		}
		std::ostream out(&file);

		EnterCriticalSection(&critSect);
		out.write(B.str().c_str(), B.str().size());
		file.close();
		B.clear();
		fprintf(stdout, "recive comand ""INT"", file :%s\n", szCurDir);
		LeaveCriticalSection(&critSect);
	}
	catch (...) {
		EnterCriticalSection(&critSect);
		B << "Error of creating file\n";
		LeaveCriticalSection(&critSect);
	}
}

DWORD WINAPI WorkWithClient(LPVOID info)
{
	tagINF inf;
	SOCKET my_sock;
	sockaddr_in my_addr;
	HANDLE hID;
	inf = ((tagINF *)info)[0];
	my_sock = *(inf.soc);
	my_addr = *(inf.caddr);

	signal(SIGINT, SignalHandler);
	char buff[1024];
	buff[0] = '\0';

	DuplicateHandle(
		GetCurrentProcess(),
		GetCurrentThread(),
		GetCurrentProcess(),
		&hID,
		0,
		FALSE,
		DUPLICATE_SAME_ACCESS);
	
	EnterCriticalSection(&critSect);

	B << "[" << hID << "]" << ":accept new client " << inet_ntoa(my_addr.sin_addr) << "\n";
	printf("[%d]:client %s accept\n", hID, inet_ntoa(my_addr.sin_addr));
	LeaveCriticalSection(&critSect);

	HANDLE hTimer = CreateWaitableTimer(NULL, FALSE, NULL);
	LARGE_INTEGER li;
	int nNanosecondsPerSecond = 1000000;
	__int64 qwTimeFromNowInNanoseconds = nNanosecondsPerSecond;

	qwTimeFromNowInNanoseconds = -qwTimeFromNowInNanoseconds,
		li.LowPart = (DWORD)(qwTimeFromNowInNanoseconds & 0xFFFFFFFF),
		li.HighPart = (LONG)(qwTimeFromNowInNanoseconds >> 32);

	SetWaitableTimer(hTimer, &li, TIMEOUT, timer_proc, (LPVOID)hID, FALSE);

	int bytes_recv;
	//ждем сособщение от клиента и записываем его в буффер
	while ((bytes_recv = recv(my_sock, &buff[0], sizeof(buff), 0))
		&& bytes_recv != SOCKET_ERROR){

			EnterCriticalSection(&critSect);
			B << buff << "\n";
			printf("recive and send string: %s\n", buff);
			LeaveCriticalSection(&critSect);
		    send(my_sock, &buff[0], bytes_recv, 0);
	}

	nclients--;
	EnterCriticalSection(&critSect);
	B << "[" << hID << "] client " << inet_ntoa(my_addr.sin_addr) << " disconnected\n";
	printf("[%d]:client %s disconnected\n", hID, inet_ntoa(my_addr.sin_addr));
	PRINTNUSERS
		LeaveCriticalSection(&critSect);

	closesocket(my_sock);
	CloseHandle(hID);
	CancelWaitableTimer(hTimer);
	ExitThread(0);
	return ACCESS_CODE;
}

VOID APIENTRY  timer_proc(LPVOID IpArgToCompletionRoutine,
	DWORD dwTimerLowValue, DWORD dwTimerHighValue)
{
	EnterCriticalSection(&critSect);
	printf("[%d]: idle\n", IpArgToCompletionRoutine);
	B << "[" << IpArgToCompletionRoutine << "]: idle\n";
	LeaveCriticalSection(&critSect);
};
