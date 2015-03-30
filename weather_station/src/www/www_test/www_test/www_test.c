// www_test.cpp : Defines the entry point for the console application.
//

#include <stdio.h>
#include <stdlib.h>
#include "website.h"

#define WWW_TX_BUFFER_LENGTH 4096
char http_server_tx_buffer[WWW_TX_BUFFER_LENGTH];

int main(int argc, char* argv[])
{
    int page_size = 0;
    char* uri = "/index.html";

    printf ("initializing pages\n");
    www_init_webpages ();

    printf ("Requested: %s\n", uri);
    page_size = www_build_response_from_uri (uri, http_server_tx_buffer);
    printf ("Send page (size %d):\n%s\n\n", page_size, http_server_tx_buffer);

    getchar ();

    return 0;
}

