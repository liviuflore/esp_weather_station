#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"

#include "httpserver-netconn.h"
#include "www/website.h"

#if LWIP_NETCONN

#define WWW_TX_BUFFER_LENGTH 4096
char http_server_tx_buffer[WWW_TX_BUFFER_LENGTH];

/** Get URI from request */
static int ICACHE_FLASH_ATTR
http_server_get_uri(char* request, int request_len, char* uri)
{
	if (request_len >= 4 && strncmp(request, "GET ", 4) == 0) {
		char* found = strstr(request, " HTTP");
		if (found != NULL) {
			int len = found - request;
			if (len > 4) {
				int page_size = 0;
				memcpy(uri, &(request[4]), len - 4);
				request[len] = '\0';
				return len;
			}
		}
	}
	return 0;
}

/** Serve one HTTP connection accepted in the http thread */
static void ICACHE_FLASH_ATTR
http_server_netconn_serve(struct netconn *conn)
{
	struct netbuf *inbuf;
	char *buf;
	u16_t buflen;
	err_t err;
	char uri[128] = { 0 };
	struct http_webpage* wp = NULL;
  
	/* Read the data from the port, blocking if nothing yet there. 
	We assume the request (the part we care about) is in one netbuf */
	err = netconn_recv(conn, &inbuf);
  
	if (err == ERR_OK) {
		netbuf_data(inbuf, (void**)&buf, &buflen);

		if (http_server_get_uri(buf, buflen, uri)) {
			int page_size = 0;
			printf("Requested: %s\n", uri);

			page_size = www_build_response_from_uri(uri, http_server_tx_buffer);
			//printf("Send page: %s\n", http_server_tx_buffer);

			if (page_size > 0) {
				netconn_write(conn, http_server_tx_buffer, page_size, NETCONN_COPY);
			}
			else{
				printf("http_server_netconn_serve: invalid response size\n");
			}
		}
		else {
			printf("http_server_netconn_serve: invalid request\n");
		}
	}
	/* Close the connection (server closes in HTTP) */
	netconn_close(conn);
  
	/* Delete the buffer (netconn_recv gives us ownership,
	so we have to make sure to deallocate the buffer) */
	netbuf_delete(inbuf);
}

/** The main function, never returns! */
static void ICACHE_FLASH_ATTR
http_server_netconn_thread(void *arg)
{
	struct netconn *conn, *newconn;
	err_t err;
	LWIP_UNUSED_ARG(arg);
  
	printf ("http_server_netconn_thread start\n");

	/* Create a new TCP connection handle */
	conn = netconn_new(NETCONN_TCP);
	if (conn == NULL) {
		printf("http_server: invalid conn");
		return;
	}
  
	/* Bind to port 80 (HTTP) with default IP address */
	netconn_bind(conn, NULL, 80);
  
	/* Put the connection into LISTEN state */
	netconn_listen(conn);
  
	do {
		err = netconn_accept(conn, &newconn);
		if (err == ERR_OK) {
			http_server_netconn_serve(newconn);
			netconn_delete(newconn);
		}
	} while(err == ERR_OK);

	printf ("http_server_netconn_thread: netconn_accept received error %d, shutting down\n", err);

	netconn_close(conn);
	netconn_delete(conn);
}

/** Initialize the HTTP server (start its thread) */
void ICACHE_FLASH_ATTR
http_server_netconn_init()
{
	printf ("http_server_netconn_init\n");
	www_init_webpages();
	xTaskCreate (http_server_netconn_thread, "httpd", 256, NULL, 2, NULL);
}

#endif /* LWIP_NETCONN*/
