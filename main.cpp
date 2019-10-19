#include <iostream>
#include <exception>
#include <curl/curl.h>
#include <fstream>
#include <exception>
#include <vector>
#include <ctime>
#include <sstream>
#include "json.hpp"

#define KEY_SECRETKEY "bHUzQN9ByXE7OpwbO0nsbxkDs:gHouABpfsAcJO8ZqL3Iuu5og7gPaep5GnFWmSabc8l8eBIAUoP"
#ifdef _WIN32
#define SHORT_SLEEP Sleep(100)
#else
#define SHORT_SLEEP usleep(100000)
#endif

using namespace std;
using json = nlohmann::json;

static size_t myCallback(void* contents, size_t size, size_t nmemb, void* userp);
static bool connectToTwitter(string& data);
static bool getAccesToken(string& userData, string& token);
static int getTweets(string& userData, int still_running, int amount, string& user);
static bool blockingGetTweets(string& userData, string& token, int amount, string& name);

int main(void){
	string userData;
	string token;

	if (connectToTwitter(userData))
		cout << "Succesfull connection to Twitter Server stablished!" << endl;
	if(getAccesToken(userData, token))
		cout << "Succesfull Access Token get! " << token << endl;
	string name = "SimoneGiertz";
	userData.clear();
	if (blockingGetTweets(userData, token, 2, name))
		cout << "Got Tweets! " << endl << userData << endl;
	return 0;
}

static int getTweets(string& userData, int still_running ,int amount, string& user) {
	CURLcode res;
	CURL* curl;
	CURLM* multiCurl;
	curl = curl_easy_init();
	multiCurl = curl_multi_init();
	if (curl && multiCurl) {

		//Le decimos a cURL que imprima todos los mensajes que se envían entre cliente y servidor.
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		//Seteamos primero la pagina donde nos vamos a conectar.
		curl_easy_setopt(curl, CURLOPT_URL, "https://api.twitter.com/oauth2/token");

		// Le decimos a cURL que trabaje con HTTP.
		curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);

		//Le decimos a cURL que cuando haya que escribir llame a myCallback
		//y que use al string readString como user data.
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, myCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &userData);

		//Seteo para seguir el redireccionamiento
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

		//Seteo el metodo de autenticacion
		curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);

		//Seteo la key y secretkey
		curl_easy_setopt(curl, CURLOPT_USERPWD, KEY_SECRETKEY);

		//Incluye header "Content-type" (De acuerdo con Twitter for Developers)
		struct curl_slist* header = NULL;
		header = curl_slist_append(header, "Content-Type: application/x-www-form-urlencoded;charset=UTF-8");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);

		//Incluye "grant_type"
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, 29L);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "grant_type=client_credentials");

		//Kcyo estaba en internet
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);


		//Atacheo el easy_handle
		curl_multi_add_handle(curl, multiCurl);

		/*long curl_timeo;
		curl_multi_timeout(curl, &curl_timeo);
		if (curl_timeo < 0)
			curl_timeo = 1000;*/

		curl_multi_perform(curl, &still_running);
		if (still_running == 0) {
			curl_multi_remove_handle(multiCurl, curl);
			curl_easy_cleanup(curl);
		}
		curl_slist_free_all(header);
	}
	return still_running;
	/* if there are still transfers, loop! */
}

static bool blockingGetTweets(string& userData, string& token, int amount, string& name) {
	bool ret = false;
	CURLcode res;
	CURL* curl;

	curl = curl_easy_init();

	if (curl) {

		//Le decimos a cURL que imprima todos los mensajes que se envían entre cliente y servidor.
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		//Seteamos primero la pagina donde nos vamos a conectar.
		curl_easy_setopt(curl, CURLOPT_URL, "https://api.twitter.com/1.1/statuses/user_timeline.json?screen_name=twitterapi&count=2");

		// Le decimos a cURL que trabaje con HTTPS.
		curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);

		//Le decimos a cURL que cuando haya que escribir llame a myCallback
		//y que use al string readString como user data.
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, myCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &userData);

		//Seteo para seguir el redireccionamiento
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

		//Seteo el metodo de autenticacion
		curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);

		//Seteo la key y secretkey
		//curl_easy_setopt(curl, CURLOPT_USERPWD, KEY_SECRETKEY);

		//-H 'Authorization: OAuth oauth_version="1.0",oauth_timestamp=??,oauth_nonce=??,oauth_signature_method="PLAINTEXT",oauth_consumer_key="??",oauth_token="??",oauth_signature="??"'
		//Incluye header "Content-type" (De acuerdo con Twitter for Developers)
		struct curl_slist* header = NULL;
		header = curl_slist_append(header, "Connection: close");
		header = curl_slist_append(header, "User-Agent: OAuth gem v0.4.4");
		header = curl_slist_append(header, "Content-Type: application/x-www-form-urlencoded;charset=UTF-8");
		header = curl_slist_append(header, "Authorization: ");
		header = curl_slist_append(header, "OAuth oauth_consumer_key=\"xvz1evFS4wEEPTGEFPHBog\",");
		header = curl_slist_append(header, "oauth_nonce=\"kYjzVBB8Y0ZFabxSWbWovY3uYSQ2pTgmZeNu2VS4cg\",");	//32 Random bytes encoded base64
		header = curl_slist_append(header, "oauth_signature=\"tnnArxj06cWHq44gCs1OSKk % 2FjLY % 3D\",");	//Signing algorythm
		header = curl_slist_append(header, "oauth_signature_method=\"HMAC - SHA1\",");
		std::time_t seconds = std::time(NULL);
		std::stringstream ss;
		ss << seconds;
		std::string ts = "oauth_timestamp=\"";
		ts += ss.str() + "\",";
		//header = curl_slist_append(header, "oauth_timestamp=\"1318622958\",");
		header = curl_slist_append(header, ts.c_str());
		header = curl_slist_append(header, "oauth_token=\"1022669580643303425-g6jU4JfiueOkmvDiReK1WWg83o0DWn\",");
		header = curl_slist_append(header, "oauth_version=\"1.0\"");
		
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
		//Kcyo estaba en internet
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

		//Con lo de abajo le decimos a curl que intente conectarse a la página

		res = curl_easy_perform(curl);
		ret = true;
		// Nos fijamos si hubo algún error
		if (res != CURLE_OK) {
			std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
			ret = false;
		}
		// Hago un clean up antes de realizar un nuevo query.
		//curl_multi_remove_handle(curl, handle);
		curl_easy_cleanup(curl);
		//curl_multi_cleanup(curl);
		curl_slist_free_all(header);
	}
	return ret;
}

static size_t myCallback(void* contents, size_t size, size_t nmemb, void* userp){
	size_t realsize = size * nmemb;
	char* data = (char*)contents;
	std::string* s = (std::string*)userp;
	s->append(data, realsize);
	return realsize;
}
//Tested. WORKS
static bool getAccesToken(string& data, string& token){
	bool ret = false;
	json j;
	ifstream i("twitterAPI.json", std::ifstream::in);
	if (i.is_open()) {
		cout << "Open json!" << endl;
		i >> j;
		i.close();
		vector<string> access_token;
		json access = j["access_token"];
		if (!access.empty()){
			access_token.push_back(access);
			token = access_token[0];
			ret = true;
		}
	}
	else
		cout << "Couldn't open twitterAPI.json" << endl;
	return ret;
}
//Tested. WORKS
static bool connectToTwitter(string& userData){
	bool ret = false;
	CURLcode res;
	CURL* curl;

	curl = curl_easy_init();

	if (curl){

		//Le decimos a cURL que imprima todos los mensajes que se envían entre cliente y servidor.
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

		//Seteamos primero la pagina donde nos vamos a conectar.
		curl_easy_setopt(curl, CURLOPT_URL, "https://api.twitter.com/oauth2/token");

		// Le decimos a cURL que trabaje con HTTP.
		curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTPS);

		//Le decimos a cURL que cuando haya que escribir llame a myCallback
		//y que use al string readString como user data.
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, myCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &userData);

		//Seteo para seguir el redireccionamiento
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

		//Seteo el metodo de autenticacion
		curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);

		//Seteo la key y secretkey
		curl_easy_setopt(curl, CURLOPT_USERPWD, KEY_SECRETKEY);

		//Incluye header "Content-type" (De acuerdo con Twitter for Developers)
		struct curl_slist* header = NULL;
		header = curl_slist_append(header, "Content-Type: application/x-www-form-urlencoded;charset=UTF-8");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header);
		
		//Incluye "grant_type"
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, 29L);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "grant_type=client_credentials");

		//Kcyo estaba en internet
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);


		//Atacheo el easy_handle
		//curl_multi_add_handle(curl, handle);

		//Con lo de abajo le decimos a curl que intente conectarse a la página
		
		res = curl_easy_perform(curl);
		ret = true;
		// Nos fijamos si hubo algún error
		if (res != CURLE_OK){
			std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
			ret = false;
		}
		// Hago un clean up antes de realizar un nuevo query.
		//curl_multi_remove_handle(curl, handle);
		curl_easy_cleanup(curl);
		//curl_multi_cleanup(curl);
		curl_slist_free_all(header);
	}
	return ret;
}
