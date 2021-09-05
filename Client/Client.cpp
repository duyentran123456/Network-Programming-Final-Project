﻿// Client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <stdlib.h>
#include <string>
#include <winsock2.h>
#include <WS2tcpip.h>
#include "gamesolution.h"
#include "utils.h"
#include <windows.h>

#define BUFF_SIZE  2048
#define MSG_EMPTY 0

#define SIGNUP "SIGNUP"
#define LOGIN  "LOGIN"
#define LOGOUT "LOGOUT"
#define START "START"
#define QUESTION "QUESTION"
#define ANSWER "ANSWER"
#define RESULT "RESULT"
#define ASSIST "ASSIST"
#define QUIT "QUIT"
#define VICTORY "VICTORY"
#define SERVER_ERROR "10"
#define BAD_REQUEST "20"
#define CANNOT_USE_ASSIST "21"
#define OUT_OF_ASSIST "22"
#define CANNOT_ANSWER "23"
#define CANNOT_QUIT "24"
#define SUCCESS "30"
#define USER_LOGINED "25"
#define ACCOUNT_LOCKED "26"
#define INCORRECT_ACCOUNT "27"
#define NO_LOGIN "28"
#define STARTED "29"

#pragma comment (lib, "Ws2_32.lib")

using namespace std;

struct Question {
	string question;
	string options[4];
};

struct Game {
	bool isPlaying;

	int level;
	int score;
	bool assist5050;
	bool assistPro;
	Question ques;

	Game() {
		this->isPlaying = false;
		this->assist5050 = true;
		this->assistPro = true;
		this->level = 1;
		this->score = 0;
	}
};
int scores[] = { 100, 400, 600, 1000, 2000, 3000, 6000, 10000, 14000, 22000, 30000, 40000, 60000, 85000, 150000 };
Game clientGame = Game();

string recv(SOCKET &currSoc);
void send(SOCKET &currSoc, char req[]);

void menu(SOCKET &client);
void gameMenu(SOCKET &client);
void logIn(SOCKET &client);
void logOut(SOCKET &client);
void signUp(SOCKET &client);
void startGame(SOCKET &client);
void renderGame(Game game);
void showInstruction(SOCKET &client);
Question getQues(SOCKET &client, int index);

int main(int argc, char* argv[])
{
	// Step 1: Inittiate WinSock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData)) {
		printf("Winsock 2.2 is not supported.\n");
		return 0;
	}

	// Step 2: Constuct socket
	SOCKET client;
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (client == INVALID_SOCKET) {
		printf("Error %d: cannot create server socket.", WSAGetLastError());
		return 0;
	}

	//Step 3: Specify server address
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(stoi(argv[2]));
	inet_pton(AF_INET, argv[1], &serverAddr.sin_addr);

	//Step 4: Request to connect server
	if (connect(client, (sockaddr *)&serverAddr, sizeof(serverAddr))) {
		printf("Error %d: Cannot connect server.", WSAGetLastError());
		return 0;
	}
	printf("Connected server!\n");

	//Step 5: Communicate with server

	menu(client);
	
	//Step 6: Close socket
	closesocket(client);
	//Step 7: Terminate Winsock
	WSACleanup();
	return 0;
}

/*
* Function to print menu
* @param client: SOCKET client
*/
void menu(SOCKET &client) {
	system("cls");
	cout << "Enter 1 to log in " << endl;
	cout << "Enter 2 to sign up" << endl;
	cout << "Enter 3 to quit" << endl;

	int select;
	do
	{
		cout << "\nChon muc muon chon: ";
		cin >> select;
		switch (select) {
		case 1: return logIn(client);
		case 2: return signUp(client);
		case 3: closesocket(client);
			exit(0);
			break;
		default:
			cout << " Chon khong hop le vui long chon lai \n";
		}
	} while (select<1 || select>3);
}

/*
* Function to print menu of game
* @param client: SOCKET client
*/
void gameMenu(SOCKET &client) {
	system("cls");
	cout << "1 - start game" << endl;
	cout << "2 - instruction" << endl;
	cout << "3 - log out" << endl;

	int select;
	do
	{
		cout << "\nChon muc muon chon: ";
		cin >> select;
		switch (select) {
		case 1: return startGame(client);
		case 2: return showInstruction(client);
		case 3: return logOut(client);
		default:
			cout << " Chon khong hop le vui long chon lai \n";
		}
	} while (select<1 || select>3);
}

/*
* Function to log in
* @param client: SOCKET client
*/
void logIn(SOCKET &client) {
	system("cls");
	string request, response;
	string username, password;

	cout << "LOGIN PAGE" << endl;
	cout << "Nhap username: " << endl; cin >> username;
	cout << "Nhap password: " << endl; cin >> password;

	request.append(LOGIN).append(" ").append(username).append(" ").append(password).append("\r\n");
	send(client, &request[0]);

	response = recv(client);
	response = split(response, "\r\n")[0];
	
	if (strcmp(&response[0], SUCCESS) != 0) {
		cout << "Error happend" << endl;
		cout << "Enter something to try again or comeback to menu" << endl;
		return menu(client);
	}

	return gameMenu(client);
}

/*
* Function to log out
* @param client: SOCKET client
*/
void logOut(SOCKET &client) {
	// send logout msg
	// cout log out success or fail and comeback to menu
	string request, response;
	request.append(LOGOUT).append("\r\n");
	send(client, &request[0]);
	response = recv(client);
	response = split(response, "\r\n")[0];
	if (strcmp(&response[0], SUCCESS) != 0) {
		cout << "Error happend" << endl;
		cout << "Enter something to try again or comeback to menu" << endl;
		return gameMenu(client);
	}
	return menu(client);
}

/*
* Function to sign up
* @param client: SOCKET client
*/
void signUp(SOCKET &client) {
	system("cls");
	string request, response;
	string username, password;

	cout << "REGISTER PAGE" << endl;
	cout << "Nhap username: " << endl; cin >> username;
	cout << "Nhap password: " << endl; cin >> password;

	request.append(SIGNUP).append(" ").append(username).append(" ").append(password).append("\r\n");
	send(client, &request[0]);

	response = recv(client);
	response = split(response, "\r\n")[0];
	vector<string> params = split(response, " ");
	if (params[0] == SUCCESS) {
		cout << "Register new user successfully, enter 1 to go to log in page, enter 2 to come back to main menu: " << endl;
		int select;
		cin >> select;
		while (select != 1 && select != 2) {
			cout << "Invalid options! Please enter 1 or 2" << endl;
			cin >> select;
		}
		if (select == 1) {
			return logIn(client);
		}
		if (select == 2) {
			return menu(client);
		}
	}
	else {
		cout << "Error " << params[0] << " - Do you want to do it again? ";
		cout << "...";
	}

}

/*
* Function to start game
* @param client: SOCKET client
*/
void startGame(SOCKET &client) {
	system("cls");
	string request, response;

	cout << "STARTING THE GAME..." << endl;
	request.append(START).append("\r\n");
	send(client, &request[0]);

	response = recv(client);
	response = split(response, "\r\n")[0];

	if (strcmp(&response[0], SUCCESS) != 0) {
		cout << "error, wanna restart?" << endl;
		cout << "....." << endl;
		return;
	}

	cout << "GETTING THE FIRST QUESTION... " << endl;
	clientGame.ques = getQues(client, 0);
	clientGame.isPlaying = true;

	int select, wrong1, wrong2, pro;
	string tmp;
	request = "";

	while (clientGame.isPlaying) {
		renderGame(clientGame);
		do
		{
			cout << "\nChon muc muon chon: ";
			cin >> select;
			switch (select) {
			case 1:
			case 2:
			case 3:
			case 4:
				request.append(ANSWER).append(" ").append(to_string(select)).append("\r\n");
				send(client, &request[0]);
				request = "";
				break;
			case 5:
				request.append(ASSIST).append(" ").append("5050").append("\r\n");
				send(client, &request[0]);
				request = "";
				break;
			case 6:
				request.append(ASSIST).append(" ").append("PRO").append("\r\n");
				send(client, &request[0]);
				request = "";
				break;
			case 7:
				request.append(QUIT).append("\r\n");
				send(client, &request[0]);
				request = "";
				break;
			default:
				cout << " Chon khong hop le vui long chon lai \n";
			}
		} while (select < 1 || select>7);

		response = recv(client);
		response = split(response, "\r\n")[0];
		vector<string> params = split(response, " ");

		if (strcmp(&(params[0])[0], SUCCESS) != 0) {
			if (strcmp(&(params[0])[0], OUT_OF_ASSIST) == 0) {
				cout << "Cannot use this assistance" << endl;
				continue;
			}
			else {
				cout << "Error ... Enter anything to come back to menu" << endl;
				string tmp; cin >> tmp;
				return menu(client);
			}
		}

		switch (select) {
		case 1:
		case 2:
		case 3:
		case 4:
			if (strcmp(&(params[1])[0], "true") == 0) {
				if (clientGame.level == 15) {
					cout << "Xin chuc mung ban da la trieu phu!" << endl;
					cout << "An phim bat ky de tro ve menu" << endl;
					cin >> tmp;
					return gameMenu(client);
				}
				cout << "Tra loi dung cau hoi!";
				clientGame.score += scores[clientGame.level-1];
				clientGame.level++;
				clientGame.ques = getQues(client, clientGame.level - 1);				
			}
			else if (strcmp(&(params[1])[0], "false") == 0) {
				clientGame.isPlaying = false;

			}
			break;
		case 5: 
			wrong1 = atoi(params[1].c_str()), wrong2 = atoi(params[2].c_str());
			clientGame.ques.options[wrong1 - 1] = "";
			clientGame.ques.options[wrong2 - 1] = "";
			clientGame.assist5050 = false;
			break;
		case 6: 
			pro = atoi(params[1].c_str());
			clientGame.ques.options[pro-1].append(" [PRO]");
			clientGame.assistPro = false;
			break;
		case 7: 
			// handle success quit msg
			clientGame.isPlaying = false;
			break;
		}
	}
	
	clientGame = Game();
	return gameMenu(client);
}

/*
* Function to render game when start success
* @param client: SOCKET client
*/
void renderGame(Game game) {
	system("cls");
	cout << "Level: " << game.level << " - " << "Score: " << game.score << endl;
	if (game.assist5050) cout << "Enter 5 to choose 5050 assistance" << endl;
	if (game.assistPro) cout << "Enter 6 to choose PRO assistance" << endl;
	cout << "\n\n";
	cout << game.ques.question << endl;
	cout << "1: " << game.ques.options[0] << endl;
	cout << "2: " << game.ques.options[1] << endl;
	cout << "3: " << game.ques.options[2] << endl;
	cout << "4: " << game.ques.options[3] << endl;

	cout << "Enter 7 to quit game" << endl;
}

/*
* Function to get a question from server
* @param client: SOCKET client
* @param index: index of question
* return a question response from server
*/
Question getQues(SOCKET &client, int index) {
	Question q;

	string request, response;
	request.append(QUESTION).append(" ").append(to_string(index)).append("\r\n");
	send(client, &request[0]);

	response = recv(client);
	response = split(response, "\r\n")[0];
	string responseCode = split(response, " ")[0];
	int pos = response.find_first_of(" ");
	response = response.substr(pos+1);
	vector<string> params = split(response, "%#%");

	if (strcmp(&responseCode[0], SUCCESS) == 0) {
		q.question = params[0];
		q.options[0] = params[1];
		q.options[1] = params[2];
		q.options[2] = params[3];
		q.options[3] = params[4];
	}
	else {
		q.question = "NULL";
		q.options[0] = "NULL";
		q.options[1] = "NULL";
		q.options[2] = "NULL";
		q.options[3] = "NULL";
	}

	return q;
}
/**
* function: show instruction
*/
void showInstruction(SOCKET &client) {
	cout << "1.  You can play as an entire class or in two groups.\n";
	cout << "If you play as an entire class, it is suggested that\n";
	cout << "you call on different students to answer questions.\n";
	cout << "If you play as two teams, you will need to play two games.\n";
	cout << "You will need to appoint a spokesperson for each team.\n";
	cout << "I have also played with two teams, alternating questions\n";
	cout << "between the two and keeping score.\n";
	cout << "2.  Once the game has loaded, and is in slide show view,\n";
	cout << "click on the $100 question.This game is very easy\n";
	cout << "to use since many of the directions are on each slide.\n";
	cout << "If a student would like to use 50 / 50, phone a friend\n";
	cout << "or ask the audience, click on the appropriate button.\n";
	cout << "Then follow the slides.\n";
	cout << "3. Directions for viewing the previous slide are given\n";
	cout << "on each slide.\n";
	cout << "4.  The game is over when a question is missed.1.You can play as an entire class or in two groups.\n";
	cout << "If you play as an entire class, it is suggested that\n";
	cout << "you call on different students to answer questions.\n";
	cout << "If you play as two teams, you will need to play two games.\n";
	cout << "You will need to appoint a spokesperson for each team.\n";
	cout << "I have also played with two teams, alternating questions\n";
	cout << "between the two and keeping score.\n";
	cout << "2.  Once the game has loaded, and is in slide show view,\n";
	cout << "click on the $100 question.This game is very easy\n";
	cout << "to use since many of the directions are on each slide.\n";
	cout << "If a student would like to use 50 / 50, phone a friend\n";
	cout << "or ask the audience, click on the appropriate button.\n";
	cout << "Then follow the slides.\n";
	cout << "3. Directions for viewing the previous slide are given\n";
	cout << "on each slide.\n";
	cout << "4.  The game is over when a question is missed.\n";

	cout << "Enter 1 to back " << endl;
	cout << "Enter 2 to quit" << endl;

	int select;
	do
	{
		cout << "\nChon muc muon chon: ";
		cin >> select;
		switch (select) {
		case 1: return gameMenu(client);
		case 2: closesocket(client);
			exit(0);
			break;
		default:
			cout << " Chon khong hop le vui long chon lai \n";
		}
	} while (select<1 || select>2);
}