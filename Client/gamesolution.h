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

	SERVER_ERROR = 10,

	BAD_REQUEST = 20,
	USERNAME_EXISTED = 21,
	OUT_OF_ASSIST = 22,
	NOT_IN_GAME = 23,
	USER_LOGINED = 25,
	ACCOUNT_LOCKED = 26,
	INCORRECT_ACCOUNT = 27,
	NO_LOGIN = 28,
	STARTED = 29,
};


/*
* Function to get detail error based on error code
* @param e: [IN] error code
* @returns a string detail of error
*/
string getErrorDetail(string e) {
	int x = atoi(e.c_str());
	switch (x) {
	case SERVER_ERROR: return "Internal server error";
	case BAD_REQUEST: return "Bad request";
	case USERNAME_EXISTED: return "Username existed";
	case OUT_OF_ASSIST: return "This assistance had been used";
	case NOT_IN_GAME: return "You are not in game yet!";
	case USER_LOGINED: return "You are already logged in!";
	case ACCOUNT_LOCKED: return "This account is blocked!";
	case INCORRECT_ACCOUNT: return "Wrong username or password!";
	case NO_LOGIN: return "You are not log in yet!";
	case STARTED: return "You already started the game!";
	default:
		return "Unknown error";
	}
}