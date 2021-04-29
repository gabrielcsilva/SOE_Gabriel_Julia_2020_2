#include <curl/curl.h>
#include <curl/easy.h>
#include <string.h>
//#include <types.hpp>
//sudo apt-get install libcurl4-openssl-dev
//g++ test_c.cpp -lcurl -o exa
#include <unistd.h>
#include <cstdio>
#include <iostream>


#define THINGSPEAK_HOST "https://api.thingspeak.com"
#define API_KEY "A9480XPE4UHD4571"

#define TIMEOUT_IN_SECS 15 // Delay between requests

int i = 0;
int j = 0; 

int main()
{
 
	// ------------------------- Curl Initialization ---------------------------
	CURLcode res = curl_global_init(CURL_GLOBAL_ALL);
	if (res != CURLE_OK) {
		std::cerr << "Curl global initialization failed, exiting" << std::endl;
		return res;
	}
	// Initialize a curl object.
	CURL *curl = curl_easy_init();
	// If the curl initialization was successful, we can start our endless loop!
	if (curl) {
		// Don't verify SSL certificates. We could probably set some HTTP headers here for a more sophisticated request.
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

		char url[128];
		int humidity=0;
		float temperature=0;

		while (humidity<=5) {

			temperature = temperature+0.5;

			humidity++;

			sprintf(url, "%s/update?api_key=%s&field1=%i&field2=%.2f", THINGSPEAK_HOST, API_KEY,
					humidity,temperature);

			// Set the URL for the curl object.
			curl_easy_setopt(curl, CURLOPT_URL, url);
			// Perform the request.
			res = curl_easy_perform(curl);

			if (res != CURLE_OK) {
				fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
			}
			// Clear bytes
			memset(url, 0, sizeof(url));

			sleep(TIMEOUT_IN_SECS);
		}

		curl_easy_cleanup(curl);

	} else {
		fprintf(stderr, "curl_easy_init() failed\n");
	}

	curl_global_cleanup();

	return 0;
}
