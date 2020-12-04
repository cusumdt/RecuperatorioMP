#include "Server.h"

#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <vector>
#include "TaTeTi.h"
#include <time.h>

//Cusumano Cristian Ariel
//03/12/2020

#pragma comment (lib, "ws2_32.lib")

#define USERNEW '0'
#define SETALIAS '1'
#define MATCH '2'
#define MAKEMOVE '3'
#define RIVALTURN '4'
#define MYTURN '5'
#define WRONGMOVE '6'
#define FINISH '7'
#define NEWGAME '8'

using namespace std;

struct Message
{
	byte cmd;
	char data[255];
};

void Server::ServerON()
{
	srand(time(NULL));

	WSADATA data;
	WORD ver = MAKEWORD(2, 2);

	Message msg;
	vector<User*> users;

	vector<TaTeTi*> roomVector;

	int wsOk = WSAStartup(ver, &data);
	if (wsOk != 0)
	{
		cerr << "Winsock no iniciado" << endl;
		return;
	}

	SOCKET listening = socket(AF_INET, SOCK_DGRAM, 0);
	if (listening == INVALID_SOCKET)
	{
		cerr << "Invalid socket" << endl;
		return;
	}

	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(8900); 
	inet_pton(AF_INET, "127.0.0.1", &hint.sin_addr);

	int bindResult = bind(listening, (sockaddr*)&hint, sizeof(hint));
	if (bindResult == SOCKET_ERROR)
	{
		cerr << "Bind no realizado" << endl;
		return;
	}

	char buf[1024];
	sockaddr_in client;
	int clientSize = sizeof(client);

	do
	{
		memset(buf, 0, sizeof(buf));
		memset(&msg, 0, sizeof(msg));

		int bytesIn = recvfrom(listening, (char*)&msg, sizeof(msg), 0, (sockaddr*)&client, &clientSize);

		if (bytesIn == SOCKET_ERROR)
		{
			cerr << "Error al recibir data" << endl;
			return;
		}

		switch (msg.cmd)
		{
		case USERNEW:
		{
			User* u = new User();
			u->id = client;
			users.push_back(u);
			cout << "Nuevo usuario iniciado" << endl;
			break;
		}
		case SETALIAS:
		{
			for (int i = 0; i < users.size(); i++)
			{
				sockaddr* clientAddress = (sockaddr*)&client;
				sockaddr* userAddress = (sockaddr*)&users[i]->id;

				if (memcmp(clientAddress, userAddress, sizeof(clientAddress)) == 0)
				{
					cout << "Nick del usuario " << i << " :" << msg.data << endl;
					strcpy_s(users[i]->alias, msg.data);
					string toSend = "Hola ";
					toSend += (char*)&msg.data;
					sendto(listening, toSend.c_str(), sizeof(toSend), 0, (sockaddr*)&users[i]->id, sizeof(users[i]->id));

					bool roomAvailable;
					roomAvailable = false;

					Message inLobbyMsg; 
					strcpy_s(inLobbyMsg.data, "Esperando a encontrar un rival");
					sendto(listening, inLobbyMsg.data, sizeof(inLobbyMsg.data), 0, (sockaddr*)&users[i]->id, sizeof(users[i]->id));

					if (roomVector.size() != 0)
					{
						for (int j = 0; j < roomVector.size(); j++)
						{
							if (roomVector[j]->CanReceivePlayer())
							{
								roomVector[j]->AddPlayer(users[i]);
								roomVector[j]->SetupPlayers();;
								roomAvailable = true;

								string msg = "Iniciando partida contra ";
								inLobbyMsg.cmd = MATCH;
								strcpy_s(inLobbyMsg.data, msg.c_str());
								strcat_s(inLobbyMsg.data, roomVector[j]->GetCurrentTurnPlayer()->alias);
								sendto(listening, (char*)&inLobbyMsg, sizeof(Message), 0, (sockaddr*)&roomVector[j]->GetNextTurnPlayer()->id, sizeof(users[i]->id));
								strcpy_s(inLobbyMsg.data, msg.c_str());
								strcat_s(inLobbyMsg.data, roomVector[j]->GetNextTurnPlayer()->alias);
								sendto(listening, (char*)&inLobbyMsg, sizeof(Message), 0, (sockaddr*)& roomVector[j]->GetCurrentTurnPlayer()->id, sizeof(users[i]->id));
								Message matchStartMsg;
								matchStartMsg.cmd = RIVALTURN;
								strcpy_s(matchStartMsg.data, " ");
								sendto(listening, (char*)&matchStartMsg, sizeof(Message), 0, (sockaddr*)&roomVector[j]->GetNextTurnPlayer()->id, sizeof(users[i]->id));
								matchStartMsg.cmd = MYTURN;
								strcpy_s(matchStartMsg.data, " ");
								sendto(listening, (char*)&matchStartMsg, sizeof(Message), 0, (sockaddr*)&roomVector[j]->GetCurrentTurnPlayer()->id, sizeof(users[i]->id));
								break;
							}
						}
						if (roomAvailable)
							break;
					}

					if (!roomAvailable)
					{
						TaTeTi* room = new TaTeTi();
						roomVector.push_back(room);
						room->AddPlayer(users[i]);
					}

					break;
				}
			}
			break;
		}
		case MAKEMOVE:
		{
			int clientMove = (int)msg.data[0] - '0';
			for (int i = 0; i < users.size(); i++)
			{
				sockaddr* clientAddress = (sockaddr*)&client;
				sockaddr* userAddress = (sockaddr*)&users[i]->id;

				if (memcmp(clientAddress, userAddress, sizeof(clientAddress)) == 0)
				{
					TaTeTi* currentRoom = users[i]->currentRoom;
					if (users[i] == currentRoom->GetCurrentTurnPlayer())
					{
						if (users[i]->currentRoom->MakeMove(clientMove))
						{
							User* nextTurn = currentRoom->GetCurrentTurnPlayer();
							Message board;
							strcpy_s(board.data, currentRoom->GetGrid().c_str());
							switch (currentRoom->GetMatchResult())
							{
							case win:
								Message secondBoard = board;
								strcat_s(board.data, "\n Ganaste");
								board.cmd = FINISH;
								sendto(listening, (char*)&board, sizeof(Message), 0, (sockaddr*)&users[i]->id, sizeof(users[i]->id));

								strcat_s(secondBoard.data, "\n Perdiste");
								secondBoard.cmd = FINISH;
								sendto(listening, (char*)&secondBoard, sizeof(Message), 0, (sockaddr*)&nextTurn->id, sizeof(nextTurn->id));
								break;
							case draw:
								board.cmd = FINISH;
								strcat_s(board.data, "\n Empate");
								sendto(listening, (char*)&board, sizeof(Message), 0, (sockaddr*)&users[i]->id, sizeof(users[i]->id));

								secondBoard.cmd = FINISH;
								sendto(listening, (char*)&board, sizeof(Message), 0, (sockaddr*)&nextTurn->id, sizeof(nextTurn->id));
								break;
							default:
								board.cmd = RIVALTURN;
								sendto(listening, (char*)&board, sizeof(Message), 0, (sockaddr*)&users[i]->id, sizeof(users[i]->id));

								board.cmd = MYTURN;
								sendto(listening, (char*)&board, sizeof(Message), 0, (sockaddr*)&nextTurn->id, sizeof(nextTurn->id));
								break;
							}
							break;
						}
					}
					Message invalid;
					invalid.cmd = WRONGMOVE;
					strcpy_s(invalid.data, " ");
					sendto(listening, (char*)&invalid, sizeof(Message), 0, (sockaddr*)&client, sizeof(client));
					break;
				}
			}
			break;
		}
		case FINISH:
			for (int i = 0; i < users.size(); i++)
			{
				sockaddr* clientAddress = (sockaddr*)&client;
				sockaddr* userAddress = (sockaddr*)&users[i]->id;

				if (memcmp(clientAddress, userAddress, sizeof(clientAddress)) == 0)
				{
					if (users[i]->currentRoom->GetOtherPlayer(users[i]) == nullptr)
					{
						for (int j = 0; j < roomVector.size(); j++)
						{
							if (roomVector[j] == users[i]->currentRoom)
							{
								delete roomVector[j];
								vector<TaTeTi*>::iterator iter = find(roomVector.begin(), roomVector.end(), roomVector[j]);
								roomVector.erase(iter);
							}
						}
					}
					if (users[i] != nullptr)
					{
						users[i]->currentRoom->ResetPlayer(users[i]);
						delete users[i];
						vector<User*>::iterator iter = find(users.begin(), users.end(), users[i]);
						users.erase(iter);
					}
				}

			}
			break;
		case NEWGAME:
			for (int i = 0; i < users.size(); i++)
			{
				sockaddr* clientAddress = (sockaddr*)&client;
				sockaddr* userAddress = (sockaddr*)&users[i]->id;

				if (memcmp(clientAddress, userAddress, sizeof(clientAddress)) == 0)
				{
					users[i]->restart = true;
					Message inLobbyMsg;
					inLobbyMsg.cmd = MATCH;
					strcpy_s(inLobbyMsg.data, "Esperando a tu rival");
					sendto(listening, (char*)&inLobbyMsg, sizeof(Message), 0, (sockaddr*)&users[i]->id, sizeof(users[i]->id));

					if (users[i]->currentRoom->GetOtherPlayer(users[i]) && users[i]->currentRoom->GetOtherPlayer(users[i])->restart)
					{
						users[i]->currentRoom->SetupPlayers();

						string msg = "Iniciando partida contra ";
						inLobbyMsg.cmd = MATCH;
						strcpy_s(inLobbyMsg.data, msg.c_str());
						strcat_s(inLobbyMsg.data, users[i]->currentRoom->GetOtherPlayer(users[i])->alias);
						sendto(listening, (char*)&inLobbyMsg, sizeof(Message), 0, (sockaddr*)&users[i]->id, sizeof(users[i]->id));

						strcpy_s(inLobbyMsg.data, msg.c_str());
						strcat_s(inLobbyMsg.data, users[i]->alias);
						sendto(listening, (char*)&inLobbyMsg, sizeof(Message), 0, (sockaddr*)&users[i]->currentRoom->GetOtherPlayer(users[i])->id, sizeof(users[i]->id));


						Message matchStartMsg;
						matchStartMsg.cmd = RIVALTURN;
						strcpy_s(matchStartMsg.data, " ");
						sendto(listening, (char*)&matchStartMsg, sizeof(Message), 0, (sockaddr*)&users[i]->currentRoom->GetNextTurnPlayer()->id, sizeof(users[i]->id));
						matchStartMsg.cmd = MYTURN;
						strcpy_s(matchStartMsg.data, " ");
						sendto(listening, (char*)&matchStartMsg, sizeof(Message), 0, (sockaddr*)&users[i]->currentRoom->GetCurrentTurnPlayer()->id, sizeof(users[i]->id));
					}
				}
			}
			break;
		default:
			sendto(listening, (char*)&"invalido", sizeof(Message), 0, (sockaddr*)&client, sizeof(client));
			break;
		}

	} while ((string)msg.data != "close");

	closesocket(listening);
	WSACleanup();

}
