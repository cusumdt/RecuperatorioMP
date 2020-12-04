#include "Client.h"


using namespace std;

#define USERNEW '0'
#define SETALIAS '1'
#define MATCH '2'
#define MAKEMOVE '3'
#define RIVALTURN '4'
#define MYTURN '5'
#define WRONGMOVE '6'
#define FINISH '7'
#define NEWGAME '8'

struct Message
{
	byte cmd;
	char data[255];
};

struct User
{
	char alias[255];
	sockaddr_in id;
};

void Client::ClientOn()
{
	WORD version = MAKEWORD(2, 2);
	WSADATA data;

	Message msg;

	if (WSAStartup(version, &data) == 0)
	{
		string ipToUse("127.0.0.1");
		int portToUse = 8900;
		sockaddr_in server;
		server.sin_family = AF_INET;
		server.sin_port = htons(portToUse);
		inet_pton(AF_INET, ipToUse.c_str(), &server.sin_addr);

		SOCKET out = socket(AF_INET, SOCK_DGRAM, 0);

		string msgtest = "";

		char buf[1024];
		int serverSize = sizeof(server);

		msg.cmd = (byte)USERNEW;
		memset(msg.data, 0, sizeof(msg.data));
		sendto(out, (char*)&msg, sizeof(Message), 0, (sockaddr*)&server, sizeof(server));

		memset(&msg, 0, sizeof(msg));

		cout << "Elige tu nick: ";

		getline(cin, msgtest);
		strcpy_s(msg.data, msgtest.c_str());

		msg.cmd = (byte)SETALIAS;
		sendto(out, (char*)&msg, sizeof(Message), 0, (sockaddr*)&server, sizeof(server));

		ReceiveData(out, server);
		ReceiveData(out, server);

		do
		{
			memset(buf, 0, sizeof(buf));
			memset(&msg, 0, sizeof(msg));

			int bytesIn = recvfrom(out, (char*)&msg, sizeof(msg), 0, (sockaddr*)&server, &serverSize);

			if (bytesIn == SOCKET_ERROR)
			{
				cerr << "Error al recibir data" << endl;
				return;
			}
			if(msg.data != " ")
				cout << msg.data << endl;

			bool shouldSendData = false;
			bool finishGame = false;
			switch (msg.cmd)
			{
			case MATCH:
				shouldSendData = true;
				break;
			case RIVALTURN:
				cout << "Turno del jugador rival" << endl;
				shouldSendData = true;
				break;
			case MYTURN:
				cout << "Tu Turno" << endl;
				break;
			case WRONGMOVE:
				cout << "Error en la jugada, vuelva a intentar" << endl;
				break;
			case FINISH:
				finishGame = true;
			default:
				break;
			}

			if (shouldSendData)
				continue;

			memset(&msg, 0, sizeof(msg));

			if (!finishGame)
			{
				cout << "Utiliza el pad num para jugar: ";
				msg.cmd = MAKEMOVE;
				getline(cin, msgtest);
				strcpy_s(msg.data, msgtest.c_str());
			}
			else
			{
				bool validInput = false;
				do
				{
					
					cout << "v para volver a jugar o s para salir" << endl;

					getline(cin, msgtest);

					if (msgtest == "v" || msgtest == "V")
					{
						strcpy_s(msg.data, msgtest.c_str());
						msg.cmd = NEWGAME;
						validInput = true;
					}
					else if (msgtest == "s" || msgtest == "S")
					{
						strcpy_s(msg.data, msgtest.c_str());
						msg.cmd = FINISH;
						validInput = true;
						close = true;
					}


				} while (!validInput);
			}

			sendto(out, (char*)&msg, sizeof(Message), 0, (sockaddr*)&server, sizeof(server));
		} while (!close);
		closesocket(out);
	}
	else
	{
		return;
	}
	WSACleanup();
}

void Client::ReceiveData(SOCKET &socket, sockaddr_in &server)
{
	char buf[1024];
	int serverSize = sizeof(server);
	int bytesIn = recvfrom(socket, buf, sizeof(buf), 0, (sockaddr*)&server, &serverSize);

	if (bytesIn == SOCKET_ERROR)
	{
		cerr << "Error al recibir data" << endl;
		return;
	}

	cout << buf << endl;
}
