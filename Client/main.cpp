#include "Client.h"

void main()
{
	Client* client = new Client();
	client->ClientOn();
	delete client;
}