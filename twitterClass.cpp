#include <iostream>
#include "twitterClass.h"


#define KEY_SECRETKEY "bHUzQN9ByXE7OpwbO0nsbxkDs:gHouABpfsAcJO8ZqL3Iuu5og7gPaep5GnFWmSabc8l8eBIAUoP"
#define TWITTER_TOKEN_URL "https://api.twitter.com/oauth2/token"


static size_t myCallback(void* contents, size_t size, size_t nmemb, void* userp);


using namespace std;
using json = nlohmann::json;

TwitterAPI::TwitterAPI() {
	error = false;
	curlEasy = curl_easy_init();
	curlMulti = curl_multi_init();
	userData.clear();
	file.clear();
	if (!connect()) {
		error = true;
	}
	currentTransfers = 0;
	return;
}
TwitterAPI::~TwitterAPI() {
	curl_multi_remove_handle(curlMulti, curlEasy);
	curl_easy_cleanup(curlEasy);
	curl_multi_cleanup(curlMulti);
	return;
}

int TwitterAPI::getTweets(string& name, int amount, int* currentTransfers) {
	CURLMcode res;
	//cout << "Entering getTweets with " << *currentTransfers << "transfers pending." << endl;
	if (*currentTransfers == 0) {
		curlMultiConfig(name, amount);
		userData.clear();
		file.clear();
	}
	res = curl_multi_perform(curlMulti, currentTransfers);
	if (*currentTransfers == 0 && !userData.empty()) {
		curl_multi_remove_handle(curlMulti, curlEasy);
		file = json::parse(userData);
	}
	/*if(!userData.empty())
		file = json::parse(userData);*/
	if (res != CURLE_OK)
		error = true;
	return *currentTransfers;
}

void TwitterAPI::curlMultiConfig(string& name, int amount) {
	//ERROR OTRO

	curl_easy_reset(curlEasy);

	string path = "https://api.twitter.com/1.1/statuses/user_timeline.json?";
	path += "screen_name=" + name + "&count=" + to_string(amount);

	string auth = "Authorization: Bearer " + token;
	userData.clear();
	if (curlMulti && curlEasy)
	{
		//Le decimos a cURL que imprima todos los mensajes que se envían entre cliente y servidor.
		//curl_easy_setopt(curlEasy, CURLOPT_VERBOSE, 1L);

		//Seteamos primero la pagina donde nos vamos a conectar.
		curl_easy_setopt(curlEasy, CURLOPT_URL, path.c_str());

		// Le decimos a cURL que trabaje con HTTP.
		curl_easy_setopt(curlEasy, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);

		//Le decimos a cURL que cuando haya que escribir llame a myCallback
		//y que use al string readString como user data.
		curl_easy_setopt(curlEasy, CURLOPT_WRITEFUNCTION, myCallback);
		curl_easy_setopt(curlEasy, CURLOPT_WRITEDATA, &userData);

		//Seteo para seguir el redireccionamiento
		curl_easy_setopt(curlEasy, CURLOPT_FOLLOWLOCATION, 1L);

		//Seteo el metodo de autenticacion
		curl_easy_setopt(curlEasy, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);

		//Incluye header "Authorization"
		struct curl_slist* header = NULL;
		header = curl_slist_append(header, auth.c_str());
		curl_easy_setopt(curlEasy, CURLOPT_HTTPHEADER, header);

		//Pra poder ver el progreso
		curl_easy_setopt(curlEasy, CURLOPT_NOPROGRESS, 0L);

		//Kcyo estaba en internet
		curl_easy_setopt(curlEasy, CURLOPT_SSL_VERIFYPEER, 0L);

		//Atacheo el easy_handle
		curl_multi_add_handle(curlMulti, curlEasy);
	}
	else
		error = true;
	return;
}
//Tested. WORKS
bool TwitterAPI::connect() {
	bool ret = false;
	CURLcode res;

	if (curlEasy) {

		//Le decimos a cURL que imprima todos los mensajes que se envían entre cliente y servidor.
		//curl_easy_setopt(curlEasy, CURLOPT_VERBOSE, 1L);

		//Seteamos primero la pagina donde nos vamos a conectar.
		curl_easy_setopt(curlEasy, CURLOPT_URL, TWITTER_TOKEN_URL);

		// Le decimos a cURL que trabaje con HTTP.
		curl_easy_setopt(curlEasy, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);

		//Le decimos a cURL que cuando haya que escribir llame a myCallback
		//y que use al string readString como user data.
		curl_easy_setopt(curlEasy, CURLOPT_WRITEFUNCTION, myCallback);
		curl_easy_setopt(curlEasy, CURLOPT_WRITEDATA, &userData);

		//Seteo para seguir el redireccionamiento
		curl_easy_setopt(curlEasy, CURLOPT_FOLLOWLOCATION, 1L);

		//Seteo el metodo de autenticacion
		curl_easy_setopt(curlEasy, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);

		//Seteo la key y secretkey
		curl_easy_setopt(curlEasy, CURLOPT_USERPWD, KEY_SECRETKEY);

		//Incluye header "Content-type" (De acuerdo con Twitter for Developers)
		struct curl_slist* header = NULL;
		header = curl_slist_append(header, "Content-Type: application/x-www-form-urlencoded;charset=UTF-8");
		curl_easy_setopt(curlEasy, CURLOPT_HTTPHEADER, header);

		//Incluye "grant_type"
		curl_easy_setopt(curlEasy, CURLOPT_POSTFIELDSIZE, 29L);
		curl_easy_setopt(curlEasy, CURLOPT_POSTFIELDS, "grant_type=client_credentials");

		//Kcyo estaba en internet
		curl_easy_setopt(curlEasy, CURLOPT_SSL_VERIFYPEER, 0L);

		//Con lo de abajo le decimos a curl que intente conectarse a la página

		res = curl_easy_perform(curlEasy);
		if (res == CURLE_OK)
		{
			//Getting acces token
			file = json::parse(userData);
			userData.clear();
			auto tkn = file["access_token"];
			userData = tkn.dump();
			userData.erase(userData.begin());
			userData.pop_back();
			token = userData;
			file.clear();
			userData.clear();
			ret = true;
			//NO HAY ERROR
		}

		curl_slist_free_all(header);
	}
	else
		ret = false;
	return ret;
}

static size_t myCallback(void* contents, size_t size, size_t nmemb, void* userp) {
	size_t realsize = size * nmemb;
	char* data = (char*)contents;
	std::string* s = (std::string*)userp;
	s->append(data, realsize);
	return realsize;
}