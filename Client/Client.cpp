// Client.cpp : Defines the entry point for the console application.
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
#define OUT_OF_ASSIST "22"
#define SUCCESS "30"

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
	cout << endl;
	cout << "\t\t\t" << "Enter 1 to log in " << endl;
	cout << "\t\t\t" << "Enter 2 to sign up" << endl;
	cout << "\t\t\t" << "Enter 3 to quit" << endl;
	
	string tmp;
	int select;
	do
	{
		cin >> tmp;
		select = atoi(tmp.c_str());
		switch (select) {
		case 1: return logIn(client);
		case 2: return signUp(client);
		case 3: closesocket(client);
			exit(0);
			break;
		default:
			cout << "Invalid options! Please enter 1, 2 or 3\n";
		}
	} while (select<1 || select>3);
}

/*
* Function to print menu of game
* @param client: SOCKET client
*/
void gameMenu(SOCKET &client) {
	system("cls");
	cout << endl;
	cout << "\t\t\t" << "1 - start game" << endl;
	cout << "\t\t\t" << "2 - instruction" << endl;
	cout << "\t\t\t" << "3 - log out" << endl;

	string tmp;
	int select;
	do
	{
		cin >> tmp;
		select = atoi(tmp.c_str());
		switch (select) {
		case 1: return startGame(client);
		case 2: return showInstruction(client);
		case 3: return logOut(client);
		default:
			cout << "Invalid options! Please enter 1, 2 or 3\n";
		}
	} while (select<1 || select>3);
}

/*
* Function to log in
* @param client: SOCKET client
*/
void logIn(SOCKET &client) {
	system("cls");
	cout << endl;
	string request, response;
	string username, password;

	cout << "\t\t\t\t" << "LOGIN PAGE" << endl;
	cout << "\t\t\t" << "Username: "; cin >> username;
	cout << "\t\t\t" << "Password: "; cin >> password;

	request.append(LOGIN).append(" ").append(username).append(" ").append(password).append("\r\n");
	send(client, &request[0]);

	response = recv(client);
	response = split(response, "\r\n")[0];
	
	if (strcmp(&response[0], SUCCESS) != 0) {
		cout << "\t\t" << getErrorDetail(&response[0]) << endl;
		cout << "\t\t" << "Enter 1 to retry, enter 2 to come back to main menu: " << endl;
		string select;
		cin >> select;
		while (strcmp(&select[0],"1")!=0 && strcmp(&select[0], "2") != 0) {
			cout << "Invalid options! Please enter 1 or 2" << endl;			
			cin >> select;
		}
		if (strcmp(&select[0], "1") == 0) {
			return logIn(client);
		}
		if (strcmp(&select[0], "2") == 0) {
			return menu(client);
		}		
	}

	return gameMenu(client);
}

/*
* Function to log out
* @param client: SOCKET client
*/
void logOut(SOCKET &client) {
	string request, response;

	request.append(LOGOUT).append("\r\n");
	send(client, &request[0]);

	response = recv(client);
	response = split(response, "\r\n")[0];

	if (strcmp(&response[0], SUCCESS) != 0) {
		cout << "\t\t" << getErrorDetail(&response[0]) << endl;
		cout << "\t\t" << "Enter anything to come back to main menu " << endl;
		string select; cin >> select;
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
	cout << endl;
	string request, response;
	string username, password;

	cout << "\t\t\t\t" << "REGISTER PAGE" << endl;
	cout << "\t\t\t" << "Username: "; cin >> username;
	cout << "\t\t\t" << "Password: "; cin >> password;

	request.append(SIGNUP).append(" ").append(username).append(" ").append(password).append("\r\n");
	send(client, &request[0]);

	response = recv(client);
	response = split(response, "\r\n")[0];
	vector<string> params = split(response, " ");
	if (params[0] == SUCCESS) {
		cout << "\t\t" << "Register new user successfully" << endl;
		cout << "\t\t" << "Enter 1 to go to log in page, enter 2 to come back to main menu : " << endl;
		string select;
		cin >> select;
		while (strcmp(&select[0], "1") != 0 && strcmp(&select[0], "2") != 0) {
			cout << "Invalid options! Please enter 1 or 2" << endl;
			cin >> select;
		}
		if (strcmp(&select[0], "1") == 0) {
			return logIn(client);
		}
		if (strcmp(&select[0], "2") == 0) {
			return menu(client);
		}
	}
	else {
		cout << "\t\t" << getErrorDetail(params[0]) << endl;
		cout << "\t\t" << "Enter 1 to retry, enter 2 to come back to main menu: " << endl;
		string select;
		cin >> select;
		while (strcmp(&select[0], "1") != 0 && strcmp(&select[0], "2") != 0) {
			cout << "Invalid options! Please enter 1 or 2" << endl;
			cin >> select;
		}
		if (strcmp(&select[0], "1") == 0) {
			return signUp(client);
		}
		if (strcmp(&select[0], "2") == 0) {
			return menu(client);
		}
	}

}

/*
* Function to start game
* @param client: SOCKET client
*/
void startGame(SOCKET &client) {
	system("cls");
	cout << endl;
	string request, response;

	cout << "STARTING THE GAME..." << endl;
	request.append(START).append("\r\n");
	send(client, &request[0]);

	response = recv(client);
	response = split(response, "\r\n")[0];

	if (strcmp(&response[0], SUCCESS) != 0) {
		cout << getErrorDetail(&response[0]) << endl;
		cout << "\t\t" << "Enter 1 to retry, enter 2 to come back to game menu: " << endl;
		string select;
		cin >> select;
		while (strcmp(&select[0], "1") != 0 && strcmp(&select[0], "2") != 0) {
			cout << "Invalid options! Please enter 1 or 2" << endl;
			cin >> select;
		}
		if (strcmp(&select[0], "1") == 0) {
			return startGame(client);
		}
		if (strcmp(&select[0], "2") == 0) {
			return gameMenu(client);
		}
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
			cin >> tmp;
			select = atoi(tmp.c_str());
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
				cout << "Invalid option! Please enter from 1 to 7 \n";
			}
		} while (select < 1 || select>7);

		response = recv(client);
		response = split(response, "\r\n")[0];
		vector<string> params = split(response, " ");

		if (strcmp(&(params[0])[0], SUCCESS) != 0) {
			if (strcmp(&(params[0])[0], OUT_OF_ASSIST) == 0) {
				cout << getErrorDetail(OUT_OF_ASSIST) << endl;
				cin >> tmp;
				continue;
			}
			else {
				cout << getErrorDetail(&(params[0])[0]) << endl;
				cout << "Enter anything to come back to menu" << endl;
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
					cout << "Congratulations! You are now a millionaire!!" << endl;
					cout << "Enter anything to come back to menu" << endl;
					cin >> tmp;
					clientGame = Game();
					return gameMenu(client);
				}
				clientGame.score += scores[clientGame.level-1];
				clientGame.level++;
				clientGame.ques = getQues(client, clientGame.level - 1);				
			}
			else if (strcmp(&(params[1])[0], "false") == 0) {
				clientGame.isPlaying = false;
				cout << "Sorry, you losed the game!" << endl;
				cout << "Enter anything to come back to menu" << endl;
				cin >> tmp;
				clientGame = Game();
				return gameMenu(client);
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
			cout << "Your score: " << clientGame.score << endl;
			cout << "Enter anything to come back to menu" << endl;
			cin >> tmp;
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
	cout << endl;
	cout << "Level: " << game.level << " - " << "Score: " << game.score << endl;
	if (game.assist5050) cout << "Enter 5 to choose 5050 assistance" << endl;
	if (game.assistPro) cout << "Enter 6 to choose PRO assistance" << endl;
	cout << "\nEnter 7 to quit game" << endl;
	cout << "\n\n";
	cout <<"\t\t" << game.ques.question << endl;
	cout << "\t\t" << "\t1: " << game.ques.options[0] << endl;
	cout << "\t\t" << "\t2: " << game.ques.options[1] << endl;
	cout << "\t\t" << "\t3: " << game.ques.options[2] << endl;
	cout << "\t\t" << "\t4: " << game.ques.options[3] << endl;	
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
	system("cls");
	cout << "This is a simple version of gameshow \"Who wants to be a millionaire\"\n" << endl;
	cout << "\t\t" << "RULES" << endl;
	cout << "There are 15 questions in total, each question has 4 options for player to choose" << endl;
	cout << "Player can use 2 assistance in a game: 5050 and PRO assistance" << endl;
	cout << "\t5050: Get rid of 2 wrong options" << endl;
	cout << "\tPRO: Ask a professional for advice, this professional has 80% of correct answer" << endl;
	cout << "Player win the game if they answer all 15 questions correctly" << endl;
	cout << "Player lose the game when they answer 1 question wrong" << endl;
	cout << "Player can quit the game, they will get the score of the current level" << endl;
	cout << "\n\n";

	cout << "Enter 1 to back " << endl;
	cout << "Enter 2 to quit" << endl;

	string tmp;
	int select;
	do
	{
		cin >> tmp;
		select = atoi(tmp.c_str());
		switch (select) {
		case 1: return gameMenu(client);
		case 2: closesocket(client);
			exit(0);
			break;
		default:
			cout << "Invalid option! Please enter 1 or 2 \n";
		}
	} while (select<1 || select>2);
}