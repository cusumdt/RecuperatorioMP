#ifndef Client_H
#define Client_H

//Cusumano Cristian Ariel
//3/12/2020

#include <iostream>
#include <WS2tcpip.h>
#include <string>

#pragma comment (lib, "ws2_32.lib")

class Client
{
	bool close = false;
public:
	void ClientOn();
	void ReceiveData(SOCKET& socket, sockaddr_in& server);
};

#endif // !Client_H

