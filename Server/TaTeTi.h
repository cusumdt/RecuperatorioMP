#ifndef TaTeTi_H
#define TaTeTi_H

#include <iostream>
#include <WS2tcpip.h>
#pragma comment (lib, "ws2_32.lib")

//Cusumano Cristian Ariel
//3/12/2020

using namespace std;

enum Cell
{
	zero, dot, cross, count
};

enum MatchState
{
	win, draw, doing
};

struct State
{
	Cell cell[9];
};

struct Node
{
	State state;
	int value;
};

struct User
{
	sockaddr_in id;
	char alias[255];
	class TaTeTi* currentRoom;
	Cell cellType;
	bool restart = false;
};

class TaTeTi
{
private:
	bool finishGame = false;
	string currentGrid;
	int turns = 9;
	State currentGamestate;
	MatchState matchResult = doing;
	User* firstPlayer = nullptr;
	User* secondPlayer = nullptr;
	User* currentTurn;
	User* nextTurn;
private:
	bool CheckGridY(State state);
	bool CheckGridX(State state);
	bool CheckGridCrosses(State state);
public:
	TaTeTi();
	void ResetGrid();
	void ResetPlayer(User*& thisPlayer);
	char CellOutput(Cell input);
	bool MakeMove(int _cell);
	void AddPlayer(User*& newPlayer);
	void SetupPlayers();
	MatchState GetMatchResult();
	string GetGrid();
	inline bool GetGameFinish() { return finishGame; }
	inline bool CheckInput(int input, State state) { return input > 0 && input <= 9 && state.cell[input - 1] == zero ? true : false; }
	inline bool CheckAll(State state) { return CheckGridX(state) || CheckGridY(state) || CheckGridCrosses(state); }
	inline bool CanReceivePlayer() { return firstPlayer && secondPlayer ? false : true; }
	inline User* GetCurrentTurnPlayer() { return currentTurn; }
	inline User* GetNextTurnPlayer() { return nextTurn; }
	inline User* GetOtherPlayer(User*& thisPlayer) { return firstPlayer == thisPlayer ? secondPlayer : firstPlayer; }
};

#endif // !TaTeTi_H

