// www_test.cpp : Defines the entry point for the console application.
//


#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include "website.h"


// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")

#define HTTP_PORT 8888
#define HTTP_BUFLEN 4096

#define WWW_TX_BUFFER_LENGTH 10240
char http_server_tx_buffer[WWW_TX_BUFFER_LENGTH];

int minihttpd_listen (int portno);

int main(int argc, char* argv[])
{
    int page_size = 0;
    char* uri = "/index.html";

    printf ("initializing pages\n");
    www_init_webpages ();

    printf ("Listen HTTP connections on %d\n", HTTP_PORT);
    int ret = minihttpd_listen (HTTP_PORT);
    printf ("Stopped. ret %d\n", ret);

    getchar ();

    return 0;
}

int minihttpd_get_uri (char* request, int request_len, char* uri)
{
    if (request_len >= 4 && strncmp (request, "GET ", 4) == 0) {
        char* found = strstr (request, " HTTP");
        if (found != NULL) {
            int len = found - request;
            if (len > 4) {
                int page_size = 0;
                memcpy (uri, &(request[4]), len - 4);
                request[len] = '\0';
                return len;
            }
        }
    }
    return 0;
}

int minihttpd_connection_serve (SOCKET *conn)
{
    int iResult;
    char uri[128] = { 0 };
    struct http_webpage* wp = NULL;
    char recvbuf[HTTP_BUFLEN];
    int recvbuflen = HTTP_BUFLEN;
    int iSendResult;

    iResult = recv (*conn, recvbuf, recvbuflen, 0);
    if (iResult > 0) {
        printf ("Bytes received: %d\n", iResult);

        if (minihttpd_get_uri (recvbuf, recvbuflen, uri)) {
            int page_size = 0;
            printf ("Requested: %s\n", uri);
            page_size = www_build_response_from_uri (uri, http_server_tx_buffer);
            //printf ("Send page: %s\n", http_server_tx_buffer);

            iSendResult = send (*conn, http_server_tx_buffer, page_size, 0);
            if (iSendResult == SOCKET_ERROR) {
                printf ("send failed with error: %d\n", WSAGetLastError ());
                return -1;
            }
            printf ("Bytes sent: %d\n", iSendResult);
        }
        else {
            printf ("failed get url\n");
            return -1;
        }
    }
    else if (iResult == 0)
        printf ("Connection closing...\n");
    else {
        printf ("recv failed with error: %d\n", WSAGetLastError ());
    }
    return iResult;
}

int minihttpd_listen (int portno)
{
    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;
    SOCKADDR_IN addr;

    // Initialize Winsock
    iResult = WSAStartup (MAKEWORD (2, 2), &wsaData);
    if (iResult != 0) {
        printf ("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    addr.sin_family = AF_INET;      // Address family
    addr.sin_port = htons (portno);   // Assign port to this socket
    addr.sin_addr.s_addr = htonl (INADDR_ANY);

    // Create a SOCKET for connecting to server
    ListenSocket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ListenSocket == INVALID_SOCKET) {
        printf ("socket failed with error: %ld\n", WSAGetLastError ());
        WSACleanup ();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind (ListenSocket, (LPSOCKADDR)&addr, sizeof (addr));
    if (iResult == SOCKET_ERROR) {
        printf ("bind failed with error: %d\n", WSAGetLastError ());
        closesocket (ListenSocket);
        WSACleanup ();
        return 1;
    }

    iResult = listen (ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf ("listen failed with error: %d\n", WSAGetLastError ());
        closesocket (ListenSocket);
        WSACleanup ();
        return 1;
    }

    while (1) {
        printf ("listening\n");
        // Accept a client socket
        ClientSocket = accept (ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            printf ("accept failed with error: %d\n", WSAGetLastError ());
            closesocket (ListenSocket);
            WSACleanup ();
            return 1;
        }

        // Receive until the peer shuts down the connection
        int res = 0;
        //do {
            printf ("serve\n");
            res = minihttpd_connection_serve (&ClientSocket);
            if (res < 0) {
                printf ("recv failed with error: %d\n", WSAGetLastError ());
                closesocket (ClientSocket);
                WSACleanup ();
                return 1;
            }

        //} while (res > 0);
        printf ("shutdown client\n");
        // shutdown the connection since we're done
        iResult = shutdown (ClientSocket, SD_SEND);
        if (iResult == SOCKET_ERROR) {
            printf ("shutdown failed with error: %d\n", WSAGetLastError ());
            closesocket (ClientSocket);
            continue;
            return 1;
        }

        // cleanup
        printf ("close client socket\n");
        closesocket (ClientSocket);
    }
    closesocket (ListenSocket);
    WSACleanup ();

    return 0;
}

