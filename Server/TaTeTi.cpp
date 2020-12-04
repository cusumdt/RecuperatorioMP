#include "TaTeTi.h"

#include "iostream"
#include <vector>
#include <cmath>

using namespace std;
//Cusumano Cristian Ariel
//3/12/2020

TaTeTi::TaTeTi()
{
	ResetGrid();
}

bool TaTeTi::CheckGridY(State state)
{
	bool opA = state.cell[6] != zero && state.cell[6] == state.cell[3] && state.cell[3] == state.cell[0];
	bool opB = state.cell[7] != zero && state.cell[7] == state.cell[4] && state.cell[4] == state.cell[1];
	bool opC = state.cell[8] != zero && state.cell[8] == state.cell[5] && state.cell[5] == state.cell[2];
	return opA||opB||opC ? true : false;
}

bool TaTeTi::CheckGridX(State state)
{
	bool opA = state.cell[6] != zero && state.cell[6] == state.cell[7] && state.cell[7] == state.cell[8];
	bool opB = state.cell[3] != zero && state.cell[3] == state.cell[4] && state.cell[4] == state.cell[5];
	bool opC = state.cell[0] != zero && state.cell[0] == state.cell[1] && state.cell[1] == state.cell[2];
	return opA || opB || opC ? true : false;
}

bool TaTeTi::CheckGridCrosses(State state)
{
	bool opA = state.cell[6] != zero && state.cell[6] == state.cell[4] && state.cell[4] == state.cell[2];
	bool opB = state.cell[8] != zero && state.cell[8] == state.cell[4] && state.cell[4] == state.cell[0];
	return opA || opB ? true : false;
}

void TaTeTi::ResetGrid()
{
	finishGame = false;
	matchResult = doing;
	turns = 9;
	for (int i = 0; i < 9; i++)
	{
		currentGamestate.cell[i] = zero;
	}
}

void TaTeTi::ResetPlayer(User*& thisPlayer)
{
	if (firstPlayer == thisPlayer)
		firstPlayer = nullptr;
	else
		secondPlayer = nullptr;
}


char TaTeTi::CellOutput(Cell input)
{
	char output;
	switch (input)
	{
	case zero:
		output = '.';
		break;
	case dot:
		output = 'O';
		break;
	case cross:
		output = 'X';
		break;
	}
	return output;
}

bool TaTeTi::MakeMove(int _cell)
{
	if (CheckInput(_cell, currentGamestate))
	{
		currentGamestate.cell[_cell - 1] = currentTurn->cellType;
		turns--;
		User* playerAux = nextTurn;
		nextTurn = currentTurn;
		currentTurn = playerAux;
		return true;
	}
	else
		return false;
}



void TaTeTi::AddPlayer(User* &newPlayer)
{
	if (firstPlayer)
	{
		secondPlayer = newPlayer;
		secondPlayer->currentRoom = this;
	}
	else 
	{
		firstPlayer = newPlayer;
		firstPlayer->currentRoom = this;
	}
}


void TaTeTi::SetupPlayers()
{
	int firstTurn = rand() % 2 + 1;
	if (firstTurn == 1)
	{
		currentTurn = firstPlayer;
		nextTurn = secondPlayer;
	}
	else
	{
		currentTurn = secondPlayer;
		nextTurn = firstPlayer;
	}
	firstPlayer->cellType = dot;
	secondPlayer->cellType = cross;
	firstPlayer->restart = false;
	secondPlayer->restart = false;
	ResetGrid();
}

MatchState TaTeTi::GetMatchResult()
{
	if (CheckAll(currentGamestate))
	{
		finishGame = true;
		return matchResult = win;
	}
	if (turns == 0)
	{
		finishGame = true;
		return matchResult = draw;
	}
	return matchResult = doing;
}
string TaTeTi::GetGrid()
{
	currentGrid = "";
	currentGrid += '\n';
	for (int i = 2; i >= 0; i--)
	{
		for (int x = 0; x < 3; x++)
		{
			currentGrid += CellOutput(currentGamestate.cell[i * 3 + x]);
			currentGrid +=  ' ';
		}
		currentGrid += '\n';
	}
	return currentGrid;
}
