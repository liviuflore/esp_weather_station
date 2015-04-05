#ifdef WIN32
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#else
#include "esp_common.h"
#endif
#include "debug.h"
#include "website.h"
#include "webpages.h"
#include "user_config.h"



static char www_404_html[] = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n<html><head><title>404</title></head><body><h1>404</h1><p>Page not found</p></body></html>";

static char www_var_buff[128];

int www_variable_get (char* url, char* response)
{
    if (!strncmp (url, "/temperature.var", sizeof ("/temperature.var") - 1)) {
        return sprintf (response, "%d", UC_GET_VAR2 (temperature));
    }
    else if (!strncmp (url, "/humidity.var", sizeof ("/humidity.var") - 1)) {
        return sprintf (response, "%d", UC_GET_VAR2 (humidity));
    }
    else if (!strncmp (url, "/update_interval.var", sizeof ("/update_interval.var") - 1)) {
        return sprintf (response, "%d", UC_GET_VAR2 (update_interval));
    }
    else if (!strncmp (url, "/wifi_ssid.var", sizeof ("/wifi_ssid.var") - 1)) {
        return sprintf (response, "%s", UC_GET_VAR2 (wifi_ssid));
    }
    else if (!strncmp (url, "/wifi_pass.var", sizeof ("/wifi_pass.var") - 1)) {
        return sprintf (response, "%s", UC_GET_VAR2 (wifi_pass));
    }
    else {
        memcpy (response, "N/A", sizeof ("N/A") - 1);
        return sizeof ("N/A") - 1;
    }
}

int www_get_response_from_uri(char* uri, char** response)
{
	char* url = NULL;
	char* wp_content = NULL;

	if (response == NULL) {
		ESP_ERR("invalid input params");
		return 0;
	}

	/* get default page */
	if (strncmp(uri, "/", 1) == 0 && strlen(uri) == 1)
		url = "/index.html";
	else if (strncmp(uri, "action?", 7) == 0) {
		url = "/index.html";
		/* parse actions */
		// www_action_set(uri);
	}
	else
		url = uri;

	int len = 0;

	if (!strncmp(url + strlen(url) - 4, ".var", 4)) {
		len = www_variable_get(url, www_var_buff);
		*response = www_var_buff;
	}
	else {
		struct www_webpage* wp = www_webpages_get(url);

		if (wp == NULL) {
			len = sizeof(www_404_html) - 1;
			*response = www_404_html;
		}
		else {
			len = wp->size;
			*response = wp->page;
		}
	}

	return len;
}


void www_init_webpages(void)
{
    www_webpages_init();

#ifdef WIN32
    UC_SET_VAR (temperature, 21);
    UC_SET_VAR (humidity, 40);
    UC_SET_VAR_STR (wifi_ssid, "ssid");
    UC_SET_VAR_STR (wifi_pass, "pass");
#endif
}


