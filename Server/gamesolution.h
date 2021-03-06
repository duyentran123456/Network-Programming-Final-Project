#pragma once

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
#define UNFORMAT_REQUEST "UNFORMAT_REQUEST"
using namespace std;

enum ResponseCode {
	SUCCESS = 30,

	SERVER_ERROR = 10,

	BAD_REQUEST = 20,
	USERNAME_EXISTED = 21,
	OUT_OF_ASSIST = 22,
	NOT_IN_GAME = 23,
	WRONG_LEVEL = 24,
	USER_LOGINED = 25,
	ACCOUNT_LOCKED = 26,
	INCORRECT_ACCOUNT = 27,
	NO_LOGIN = 28,
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
	sockaddr_in clientAddr;
	Account userClient;
	int statusLogin;// logined is 1, else 0
	int statusInGame; // 0, 1, ...15/ -1, default = -1
	vector<Question> listQues;
	Assist assist;
};
/*
* Function to get all accounts in database
* @params accounts_path: path of file account
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
		account.status = stoi(data[2]);
		accounts.push_back(account);
	}
	file.close();
	return accounts;
}

/*
* Function save new account to database
* @param account: account want to save
* @param accounts_path: path of file account
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
* @params questions_path: path of file questions
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


/*
* Function to sign up
* @params clientInfo: current ClientInfo
* @params body: payload request from client
* @returns string response to send client
*/
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

/*
* Function to log in
* @params clientInfo: current ClientInfo
* @params body: payload request from client
* @returns string response to send client
*/
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

/*
* Function to log out
* @params clientInfo: current ClientInfo
* @returns string response to send client
*/
string logOut(ClientInfo* clientInfo) {

	ResponseCode response;
	if (!clientInfo->statusLogin) {
		response = NO_LOGIN;
	}
	else {
		clientInfo->statusLogin = false;
		clientInfo->statusInGame = -1;
		clientInfo->listQues.clear();
		clientInfo->assist.proChoice = false;
		clientInfo->assist._50_50 = false;
		response = SUCCESS;
	}

	return to_string(response);
}

/*
* Function to start game
* @params clientInfo: current ClientInfo
* @params body: payload request from client
* @returns string response to send client
*/
string start(ClientInfo* clientInfo, char *body) {
	ResponseCode response = BAD_REQUEST;
	if (!clientInfo->statusLogin) {
		response = NO_LOGIN;
	}
	else {
		if (clientInfo->statusInGame != -1) {
			response = STARTED;
		}
		else {
			clientInfo->statusInGame = 0;
			clientInfo->assist = Assist();
			int len = questions.size();
			int rands = random(1, len - 14);
			for (int i = 0; i < 15; i++) {
				clientInfo->listQues.push_back(questions[i + rands - 1]);
			}
			response = SUCCESS;
		}
	}
	return to_string(response);
}

/*
* Function to get a question from list questions
* @params clientInfo: current ClientInfo
* @params body: payload request from client
* @returns string response to send client
*/
string getQues(ClientInfo* clientInfo, char *body) {
	string bodyStr = string(body);
	string response;
	ResponseCode responseCode;
	string responseInfo = "";
	if (!clientInfo->statusLogin) responseCode = NO_LOGIN;
	else if (clientInfo->statusInGame == -1) responseCode = NOT_IN_GAME;
	else if (clientInfo->statusInGame != stoi(bodyStr)) {
		responseCode = WRONG_LEVEL;
	}
	else {
		responseCode = SUCCESS;
		Question q = clientInfo->listQues[stoi(bodyStr)];
		responseInfo.append(q.question).append("%#%").append(q.options[0]).append("%#%").append(q.options[1]).append("%#%").append(q.options[2]).append("%#%").append(q.options[3]);
	}

	response.append(to_string(responseCode)).append(" ").append(responseInfo);
	return response;
}

/*
* Function to answer a question
* @params clientInfo: current ClientInfo
* @params body: payload request from client
* @returns string response to send client
*/
string answer(ClientInfo* clientInfo, char *body) {

	string response;
	ResponseCode responseCode;
	string responseInfo = "";


	if (!clientInfo->statusLogin) responseCode = NO_LOGIN;
	else if (clientInfo->statusInGame == -1) responseCode = NOT_IN_GAME;
	else {
		responseCode = SUCCESS;
		int qNum = clientInfo->statusInGame;
		if (clientInfo->listQues[qNum].answer.compare(body) == 0) {
			responseInfo = "true";
			if (qNum == 14) {
				clientInfo->statusInGame = -1;
			}
			else clientInfo->statusInGame++;
		}
		else {
			responseInfo = "false";
			clientInfo->statusInGame = -1;
		}
	}

	response.append(to_string(responseCode));
	if (responseInfo.length() != 0)	response.append(" ").append(responseInfo);


	return response;
}

/*
* Function to get an assistant
* @params clientInfo: current ClientInfo
* @params body: payload request from client
* @returns string response to send client
*/
string assist(ClientInfo* clientInfo, char *body) {

	string response;
	ResponseCode responseCode;
	string responseInfo;


	if (!clientInfo->statusLogin) responseCode = NO_LOGIN;
	else if (clientInfo->statusInGame == -1) responseCode = NOT_IN_GAME;
	else if (strcmp(body, "5050") == 0) {
		if (clientInfo->assist._50_50) {
			responseCode = SUCCESS;
			clientInfo->assist._50_50 = false;

			string options[4] = { "1" , "2", "3", "4" };
			string rightAns = clientInfo->listQues[clientInfo->statusInGame].answer;
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

			string options[4] = { "1" , "2", "3", "4" };
			int random = rand() % 100;
			string proChoice = random < 80 ? clientInfo->listQues[clientInfo->statusInGame].answer : options[random % 4];

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

/*
* Function to quit game
* @params clientInfo: current ClientInfo
* @params body: payload request from client
* @returns string response to send client
*/
string quit(ClientInfo* clientInfo, char *body) {
	ResponseCode response = BAD_REQUEST;
	if (!clientInfo->statusLogin) {
		response = NO_LOGIN;
	}
	else {
		if (clientInfo->statusInGame == -1) {
			response = NOT_IN_GAME;
		}
		else {
			clientInfo->statusInGame = -1;
			clientInfo->listQues.clear();
			clientInfo->assist.proChoice = false;
			clientInfo->assist._50_50 = false;
			response = SUCCESS;
		}
	}
	return to_string(response);
}

/*
* Function to solve request from client
* @params clientInfo: current ClientInfo
* @params str: string message request from client
* @returns string response to send client
*/
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
	else if (strcmp(&header[0], QUESTION) == 0) {
		response = getQues(clientInfo, &payload[0]);
	}
	else response = UNFORMAT_REQUEST;
	return response;
}

/*
* Function to split messages request by "\r\n"
* @params s: string need to split
* @returns vector string splited
*/
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

/*
* Function to communicating with client
* @params clientInfo: current ClientInfo
*/
void communicating(ClientInfo *clientInfo) {

	string buff;
	string res;
	buff = recv(clientInfo->socket);
	if (buff.size() == 0) {
		return;
	}

	buff[buff.size() - 2] = 0;
	
	string tmp;
	for (int i = 0; i < buff.size() - 2; i++) tmp.push_back(buff[i]);
	vector<string> reqs = splitRequest(buff);
	
	for (int i = 0; i < reqs.size(); i++) {
		string req = reqs[i];
		res = solveRequest(clientInfo, &req[0]);
		res.append("\r\n");
		send(clientInfo->socket, &res[0]);
	}
}

