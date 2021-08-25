#pragma once
// include function of game 

#include <string>
#include <vector>
#include "fstream"
#include "time.h"
#include "stdio.h"
#include "iostream"
#include "utils.h"
#include "ws2tcpip.h"
#include "winsock2.h"
#define BUFF_SIZE 2048

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
#define UNFORMAT_REQUEST "UNFORMAT_REQUEST"
using namespace std;

enum ResponseCode {
	SUCCESS = 30,
	// Login
	USER_LOGINED = 25,
	ACCOUNT_LOCKED = 26,
	INCORRECT_ACCOUNT = 27,
	NO_LOGIN = 28,

	SERVER_ERROR = 10,
	BAD_REQUEST = 20,
	USERNAME_EXISTED = 21,
	OUT_OF_ASSIST = 22,
	NOT_IN_GAME = 23,
	CANNOT_QUIT = 24,
	STARTED = 29,
};

struct Question {
	int id;
	string question;
	string options[4];
	string answer;
};
struct Assist {
	bool _50_50;
	bool proChoice;
	Assist() {
		this->_50_50 = true;
		this->proChoice = true;
	}
};

struct Account {
	string username;
	string password;
	int status;
};

struct ClientInfo {
	SOCKET socket;
	sockaddr_in clientAddr;// address of client
	Account userClient; // user client logined
	int statusLogin;// logined is 1, else 0
	int statusInGame;
	Question listQues[15];
	int score;
	int currQuestion;
	Assist assist;
};
/*
* Function to get all accounts in database
* @returns list of account in database
*/
vector<Account> getAllAccounts(string accounts_path) {
	vector<Account> accounts;
	string line;
	ifstream file(accounts_path);
	while (getline(file, line))
	{
		cout << "line : " << line << endl;
		Account account;
		vector<string> data = split(line, " ");
		account.username = data[0];
		account.password = data[1];
		account.status = stoi(data[2]); // convert to int
		accounts.push_back(account);
	}
	file.close();
	return accounts;
}

/*
* Function save new account to database
*/
void saveAccount(Account account, string accounts_path) {
	string input;
	input.append(account.username);
	input.append(" ");
	input.append(account.password);
	input.append(" ");
	input.append(to_string(account.status));

	// write file
	ofstream file(accounts_path, ios::app);
	file << input << endl;
	file.close();
}

/*
* Function to get all questions in database
* @returns list of question in database
*/
vector<Question> getAllQuestions(string questions_path) {
	vector<Question> questions;
	int id = 1;
	string line;
	ifstream file(questions_path);
	while (getline(file, line))
	{
		Question question;
		question.id = id;
		question.question = line;
		for (int i = 0; i < 5; i++) {
			getline(file, line);
			if (i != 4) {
				question.options[i] = line;
			}
			else {
				question.answer = line;
			}
		}
		questions.push_back(question);
		id++;
	}
	file.close();
	return questions;
}
string QUESTIONS_PATH = "questions.txt";
string ACCOUNTS_PATH = "account.txt";

vector<Account> accounts = getAllAccounts(ACCOUNTS_PATH);
vector<Question> questions = getAllQuestions(QUESTIONS_PATH);

string signUp(ClientInfo* clientInfo, char *body) {

	ResponseCode response;
	Account account;
	vector<string> data = split(body, " ");
	account.username = data[0];
	account.password = data[1];
	account.status = 1;

	if (clientInfo->statusLogin) {
		response = USER_LOGINED;
	}
	else {
		int number_accounts = accounts.size();
		int i;
		for (i = 0; i < number_accounts; i++) {
			if (accounts[i].username == account.username) {
				response = USERNAME_EXISTED;
				break;
			}
		}
		if (i == number_accounts) {
			saveAccount(account, ACCOUNTS_PATH);
			accounts.push_back(account);
			response = SUCCESS;
		}
	}

	return to_string(response);
}
string logIn(ClientInfo* clientInfo, char* body) {
	ResponseCode response = INCORRECT_ACCOUNT;
	Account account;
	vector<string> data = split(body, " ");
	account.username = data[0];
	account.password = data[1];
	if (clientInfo->statusLogin) {
		response = USER_LOGINED;
	}
	else {
		int number_accounts = accounts.size();
		for (int i = 0; i < number_accounts; i++) {
			if (accounts[i].username == account.username && accounts[i].password == account.password)
			{
				if (accounts[i].status == 1) {
					clientInfo->statusLogin = true;
					clientInfo->userClient.username = account.username;
					response = SUCCESS;
				}
				else {
					response = ACCOUNT_LOCKED;
				}
				break;
			}
		}
	}
	return to_string(response);
}
string logOut(ClientInfo* clientInfo) {

	ResponseCode response;
	if (!clientInfo->statusLogin) {
		response = NO_LOGIN;
	}
	else {
		clientInfo->statusLogin = false;
		clientInfo->statusInGame = false;
		response = SUCCESS;
	}

	return to_string(response);
}
string start(ClientInfo* clientInfo, char *body) {
	ResponseCode response = BAD_REQUEST;
	if (!clientInfo->statusLogin) {
		response = NO_LOGIN;
	}
	else {
		if (clientInfo->statusInGame) {
			response = STARTED;
		}
		else {
			// start game
			clientInfo->statusInGame = true;
			clientInfo->score = 0;
			clientInfo->currQuestion = 1;
			clientInfo->assist = Assist();
			//clientInfo->listQues = ;
			response = SUCCESS;
		}
	}
	return to_string(response);
}
string answer(ClientInfo* clientInfo, char *body) {

	string response;
	ResponseCode responseCode;
	string responseInfo = "";


	if (!clientInfo->statusLogin) responseCode = NO_LOGIN;
	else if (!clientInfo->statusInGame) responseCode = NOT_IN_GAME;
	else {
		responseCode = SUCCESS;
		int qNum = clientInfo->currQuestion;
		if (clientInfo->listQues[qNum].answer.compare(body) == 0) responseInfo = "true";
		else responseInfo = "false";
	}

	response.append(to_string(responseCode));
	if (responseInfo.length() != 0)	response.append(" ").append(responseInfo);


	return response;
}
string assist(ClientInfo* clientInfo, char *body) {

	string response;
	ResponseCode responseCode;
	string responseInfo;


	if (!clientInfo->statusLogin) responseCode = NO_LOGIN;
	else if (!clientInfo->statusInGame) responseCode = NOT_IN_GAME;
	else if (strcmp(body, "5050") == 0) {
		if (clientInfo->assist._50_50) {
			responseCode = SUCCESS;
			clientInfo->assist._50_50 = false;

			string options[4] = { "A" , "B", "C", "D" };
			string rightAns = clientInfo->listQues[clientInfo->currQuestion].answer;
			int wrong1, wrong2, right, random;
			for (int i = 0; i < 4; i++)
				if (rightAns.compare(options[i]) == 0) {
					right = i;
					break;
				}
			random = rand() % 4;
			while (random == right) random = rand() % 4;
			wrong1 = random;
			while (random == right || random == wrong1) random = rand() % 4;
			wrong2 = random;

			responseInfo.append(options[wrong1]).append(" ").append(options[wrong2]);
		}
		else responseCode = OUT_OF_ASSIST;
	}
	else if (strcmp(body, "PRO") == 0) {
		if (clientInfo->assist.proChoice) {
			responseCode = SUCCESS;
			clientInfo->assist.proChoice = false;

			string options[4] = { "A" , "B", "C", "D" };
			int random = rand() % 100;
			string proChoice = random < 80 ? clientInfo->listQues[clientInfo->currQuestion].answer : options[random % 4];

			responseInfo.append(proChoice);
		}
		else responseCode = OUT_OF_ASSIST;
	}
	else {
		responseCode = BAD_REQUEST;
	}

	response.append(to_string(responseCode));
	if (responseInfo.length() != 0)	response.append(" ").append(responseInfo);

	return response;
}
string quit(ClientInfo* clientInfo, char *body) {
	ResponseCode response = BAD_REQUEST;
	if (!clientInfo->statusLogin) {
		response = NO_LOGIN;
	}
	else {
		if (!clientInfo->statusInGame) { // no in game
			response = CANNOT_QUIT;
		}
		else {
			// quit game
			clientInfo->statusInGame = false;
			clientInfo->score = 0;
			//clientInfo->listQues = ;
			response = SUCCESS;
		}
	}
	return to_string(response);
}


string solveRequest(ClientInfo *clientInfo, char str[]) {
	string header, payload;
	string convertStr = convertToString(str, strlen(str));
	vector<string> requests = split(convertStr, " ");
	header = requests[0];
	for (int i = header.length() + 1; i < strlen(str); i++) {
		payload.push_back(str[i]);
	}
	string response;
	if (strcmp(&header[0], LOGIN) == 0) {
		response = logIn(clientInfo, &payload[0]);
	}
	else if (strcmp(&header[0], START) == 0) {
		response = start(clientInfo, &payload[0]);
	}
	else if (strcmp(&header[0], QUIT) == 0) {
		response = quit(clientInfo, &payload[0]);
	}
	else if (strcmp(&header[0], SIGNUP) == 0) {
		response = signUp(clientInfo, &payload[0]);
	}
	else if (strcmp(&header[0], LOGOUT) == 0) {
		response = logOut(clientInfo);
	}
	else if (strcmp(&header[0], ANSWER) == 0) {
		response = answer(clientInfo, &payload[0]);
	}
	else if (strcmp(&header[0], ASSIST) == 0) {
		response = assist(clientInfo, &payload[0]);
	}

	else response = UNFORMAT_REQUEST;
	return response;
}

vector<string> splitRequest(string s) {
	vector<string> res;
	string delimiter = "\r\n";
	size_t pos = 0;
	string token;
	while ((pos = s.find(delimiter)) != string::npos) {
		token = s.substr(0, pos);
		res.push_back(token);
		s.erase(0, pos + delimiter.length());
	}
	if (s.length()>0) res.push_back(s);
	return res;
}

void communicating(ClientInfo *clientInfo) {
	// receive message from client
	string buff;
	string res;
	buff = recv(clientInfo->socket);
	if (buff.size() == 0) {
		//string tmp = quit(clientInfo);
		return;
	}

	buff[buff.size() - 2] = 0;
	// copy message without "\r\n"
	string tmp;
	for (int i = 0; i < buff.size() - 2; i++) tmp.push_back(buff[i]);
	vector<string> reqs = splitRequest(buff);
	// solve each string of vector string request
	for (int i = 0; i < reqs.size(); i++) {
		string req = reqs[i];
		cout << "req" << req;
		res = solveRequest(clientInfo, &req[0]);
		res.append("\r\n");
		// send response after append "\r\n" to client
		send(clientInfo->socket, &res[0]);
		cout << "res" << res;
	}
}

