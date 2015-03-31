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

#include "user_config.h"
#include "httpserver-netconn.h"

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

