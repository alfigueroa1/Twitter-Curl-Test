#pragma once

#include <curl/curl.h>
#include <string>
#include "json.hpp"

using namespace std;
using json = nlohmann::json;

class TwitterAPI {
public:
	TwitterAPI();
	~TwitterAPI();
	bool getError() { return error; }
	void clearError() { error = false; return; };
	int getTransfers() { return currentTransfers; }
	json getJSON() { return file; }
	int getTweets(string& name, int amount, int* currentTransfers);

private:
	bool connect();
	void curlMultiConfig(string& name, int amount);
	bool error;
	int currentTransfers;
	CURLM* curlMulti;
	CURL* curlEasy;
	json file;
	string userData;
	string token;
};
