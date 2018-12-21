#include "stdafx.h"
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <string.h>
#include <time.h>
#include <iostream>

#pragma comment(lib, "ws2_32.lib")			 
#define ERROR_WSASTARTUP 1
#define ERROR_SOCKET 2
#define ERROR_CONNECT -1
#define ERROR_ADDRESS 3
#define ACCESS_CODE 0
#define RECV_ERROR 4
#define PORT 5223

FILE *mylog;

int main(int argc, char* argv[])
{
	char buff[1024];
	const char* SERVERADDR = argv[1];

	if (WSAStartup(0x201, (WSADATA *)&buff[0]))
	{
		return ERROR_WSASTARTUP;
	}

	SOCKET my_sock;
	if ((my_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		WSACleanup();
		return ERROR_SOCKET;
	}

	sockaddr_in dest_addr;
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(PORT);
	HOSTENT *hst = NULL;


	if (inet_addr(SERVERADDR) != INADDR_NONE)
		dest_addr.sin_addr.s_addr = inet_addr(SERVERADDR);
	else

		if (hst = gethostbyname(SERVERADDR))
			((unsigned long *)&dest_addr.sin_addr)[0] =
			((unsigned long **)hst->h_addr_list)[0][0];
		else
		{
			closesocket(my_sock);
			WSACleanup();
			return ERROR_ADDRESS;
		}

	if (connect(my_sock, (sockaddr *)&dest_addr,
		sizeof(dest_addr)))
	{
		return ERROR_CONNECT;
	}

	//засыпаем
	long randomNumber;

	randomNumber = 2 + rand() % 9;

	_sleep(1000 * randomNumber);


	const char charset[] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	srand(time(NULL));

	for (int i = 0; i < 10; i++){
		buff[i] = charset[rand() % strlen(charset)];

    }
	int nsize;
	buff[11] = 0;
	std::cout << buff;

	send(my_sock, &buff[0], sizeof(buff) - 1, 0);

	if ((nsize = recv(my_sock, &buff[0],
		sizeof(buff) - 1, 0))
		!= SOCKET_ERROR)
	{
		_sleep(1000 * randomNumber);
		closesocket(my_sock);
		WSACleanup();
		return ACCESS_CODE;
	}
	closesocket(my_sock);
	WSACleanup();
	return WSAGetLastError();
}

