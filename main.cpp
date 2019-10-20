#include <iostream>
#include <exception>
#include <curl/curl.h>
#include <exception>
#include "json.hpp"

#include "twitterClass.h"

using namespace std;
using json = nlohmann::json;

static void printTweets(TwitterAPI& twitter, string name, int amount);

int main(void){
	TwitterAPI twitter;
	if (twitter.getError()) {
		cout << "Error initializing TwitterAPI!" << endl;
		return 0;
	}

	printTweets(twitter, "thephysicsgirl", 10);
	printTweets(twitter, "SimoneGiertz", 7);
	return 0;
}

void printTweets(TwitterAPI& twitter, string name, int amount) {
	int running = 0;
	cout << endl << "Tweets form " << name << " :" << endl << endl;
	//Aca esta hecho don do..while, pero en realidad se deberia llamar lo de adentro solo cuando no hay mas eventos para resolver
	//Si se quiere parar el proceso de bajada de tweets, creo que bastaria con running = 0 fuera de la funcion y no llamarla mas.
	do {
		twitter.getTweets(name, amount, &running);
		if (twitter.getError()) {
			cout << "Error in tweets transfer!" << endl;
			break;
		}
	} while (running != 0);

	//cout << "Got all tweets!" << endl;
	vector<string> tweets;
	json file = twitter.getJSON();
	for (auto& element : file) {
		auto twt = element["text"];
		tweets.push_back(twt);
	}
	for (int i = 0; i < tweets.size(); i++)
		cout << tweets.at(i) << endl;
}