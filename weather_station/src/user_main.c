/******************************************************************************
 * Copyright 2013-2014 Espressif Systems (Wuxi)
 *
 * FileName: user_main.c
 *
 * Description: entry file of user application
 *
 * Modification history:
 *     2014/12/1, v1.0 create this file.
*******************************************************************************/
#include "esp_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "udhcp/dhcpd.h"

#include "httpserver-netconn.h"

#define MYSSID "skynet"
#define MYPASSPHRASE  "terminator.3"

#define server_ip "192.168.101.142"
#define server_port 9669


/* Global: Our IP address */
struct ip_info ipinfo;

xSemaphoreHandle semConnect = NULL;

void task2 (void *pvParameters);
void task3 (void *pvParameters);

/******************************************************************************
* FunctionName : connect_task
* Description  : Waits for connection and for an IP address before starting the other tasks.
*                Keeps checking for connection afterwards.
* Parameters   : none
* Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR connect_task (void *pvParameters)
{
    portTickType xLastWakeTime = xTaskGetTickCount ();
    portTickType xFrequency = 1000;             // Every 10 s
    struct station_config stationConf;

    unsigned char cnt = 0;
    unsigned char isinit = 0;

    memset (&stationConf, 0, sizeof (stationConf));
    strcpy (stationConf.ssid, MYSSID);
    strcpy (stationConf.password, MYPASSPHRASE);

    //Set station mode
    wifi_set_opmode (STATION_MODE);
    wifi_station_set_config (&stationConf);

    for (;;) {
        // Wait for the next cycle.
        vTaskDelayUntil (&xLastWakeTime, xFrequency);
        uint8 connect_status = wifi_station_get_connect_status ();
        if (connect_status == STATION_GOT_IP) {      // Got an IP?
            wifi_get_ip_info (0, &ipinfo);                               // Get it

            if (!isinit) {                                              // Start the tasks once we have our IP address
                char *ipptr = (char *)&ipinfo.ip;
                printf ("Got IP addr [%d.%d.%d.%d]\n", ipptr[0], ipptr[1], ipptr[2], ipptr[3]);
                isinit = 1;
                xFrequency = 6000;                  // Once a minute from now on
                if (semConnect != NULL)
                    xSemaphoreGive (semConnect);
            }
        }
        else {                                      // Lost our connection?
            printf ("Waiting for IP address [connect status %d] - %d\n", connect_status, cnt);
            xFrequency = 1000;                      // Check every 10s

            if (cnt++ > 50) {                       // Try to reconnect
                printf ("Reconnecting...\n");
                wifi_station_disconnect ();
                vTaskDelay (100);
                wifi_station_connect ();
                cnt = 0;
            }
        }
    }
}


/******************************************************************************
* FunctionName : main_task
* Description  : Does all the other stuff
* Parameters   : none
* Returns      : none
*******************************************************************************/

void main_task (void *pvParameters)
{
    // Waiting for connect semaphore
    while (xSemaphoreTake (semConnect, portMAX_DELAY) != pdTRUE);
    printf ("CONNECTED \n");

    //xTaskCreate(task2, "tsk2", 256, NULL, 2, NULL);
    //xTaskCreate(task3, "tsk3", 256, NULL, 2, NULL);
	http_server_netconn_init();

    while (1);
}

void task2(void *pvParameters)
{
    printf ("Waiting for connect semaphore\n");
    while (xSemaphoreTake (semConnect, portMAX_DELAY) != pdTRUE);
    printf ("CONNECTED!!!!!!!!!!!!!!\n");

    printf("Hello, welcome to client!\r\n");


    while (1) {
        int recbytes;
        int sin_size;
        int str_len;
        int sta_socket;

        struct sockaddr_in local_ip;
        struct sockaddr_in remote_ip;

        sta_socket = socket(PF_INET, SOCK_STREAM, 0);

        if (-1 == sta_socket) {
            close(sta_socket);
            printf("C > socket fail!\n");
            continue;
        }

        printf("C > socket ok!\n");
        bzero(&remote_ip, sizeof(struct sockaddr_in));
        remote_ip.sin_family = AF_INET;
        remote_ip.sin_addr.s_addr = inet_addr(server_ip);
        remote_ip.sin_port = htons(server_port);

        if (0 != connect(sta_socket, (struct sockaddr *)(&remote_ip), sizeof(struct sockaddr))) {
            close(sta_socket);
            printf("C > connect fail!\n");
            vTaskDelay(4000 / portTICK_RATE_MS);
            continue;
        }

        printf("C > connect ok!\n");
        char *pbuf = (char *)zalloc(1024);
        sprintf(pbuf, "%s\n", "client_send info");

        if (write(sta_socket, pbuf, strlen(pbuf) + 1) < 0) {
            printf("C > send fail\n");
        }

        printf("C > send success\n");
        free(pbuf);

        char *recv_buf = (char *)zalloc(128);
        while ((recbytes = read(sta_socket , recv_buf, 128)) > 0) {
        	recv_buf[recbytes] = 0;
            printf("C > read data success %d!\nC > %s\n", recbytes, recv_buf);
        }
        free(recv_buf);

        if (recbytes <= 0) {
            printf("C > read data fail!\n");
        }
    }
}

void task3(void *pvParameters)
{
    while (1) {
        struct sockaddr_in server_addr, client_addr;
        int server_sock, client_sock;
        socklen_t sin_size;
        bzero(&server_addr, sizeof(struct sockaddr_in));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(80);

        int recbytes;

        do {
            if (-1 == (server_sock = socket(AF_INET, SOCK_STREAM, 0))) {
                printf("S > socket error\n");
                break;
            }

            printf("S > create socket: %d\n", server_sock);

            if (-1 == bind(server_sock, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr))) {
                printf("S > bind fail\n");
                break;
            }

            printf("S > bind port: %d\n", ntohs(server_addr.sin_port));

            if (-1 == listen(server_sock, 5)) {
                printf("S > listen fail\n");
                break;
            }

            printf("S > listen ok\n");

            sin_size = sizeof(client_addr);

            for (;;) {
                printf("S > wait client\n");

                if ((client_sock = accept(server_sock, (struct sockaddr *) &client_addr, &sin_size)) < 0) {
                    printf("S > accept fail\n");
                    continue;
                }

                printf("S > Client from %s %d\n", inet_ntoa(client_addr.sin_addr), htons(client_addr.sin_port));

                char *recv_buf = (char *)zalloc(128);
                while ((recbytes = read(client_sock , recv_buf, 128)) > 0) {
                	recv_buf[recbytes] = 0;
                    printf("S > read data success %d!\nS > %s\n", recbytes, recv_buf);
                }
                free(recv_buf);

                if (recbytes <= 0) {
                    printf("S > read data fail!\n");
                    close(client_sock);
                }
            }
        } while (0);
    }
}

/******************************************************************************
 * FunctionName : user_init
 * Description  : entry of user application, init user function here
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void ICACHE_FLASH_ATTR
user_init(void)
{
    printf("SDK version:%d.%d.%d\n",
    		SDK_VERSION_MAJOR,
    		SDK_VERSION_MINOR,
    		SDK_VERSION_REVISION);

    //memset (&ipinfo, 0, sizeof (ipinfo));
    vSemaphoreCreateBinary (semConnect);
    if (semConnect == NULL) {
        printf ("failed to create semaphore\n");
        return;
    }
    xSemaphoreTake (semConnect, (portTickType)10);

    printf ("start Connecting task\n");

    // start connect task and wait to be connected to AP
    xTaskCreate (connect_task, "conn", 256, NULL, 2, NULL);
    xTaskCreate (main_task, "main", 256, NULL, 2, NULL);
}
