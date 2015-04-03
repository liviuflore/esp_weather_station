#ifdef WIN32
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#else
#include "esp_common.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "gpio.h"
#endif
#include "debug.h"


#ifdef WIN32
int DHT_init (void) { return 1; }
int DHT_get_temp () { return 26; }
int DHT_get_hum () { return 80; }
#else

#define DHT_READ_INTERVAL 1000
#define MAXTIMINGS 10000
#define BREAKTIME 20

static void ICACHE_FLASH_ATTR DHT_read_timeout (void *pvParameters);

static xSemaphoreHandle DHT_read_sem = NULL;
static volatile os_timer_t DHT_read_timer;
static int DHT_temp = 0;
static int DHT_hum = 0;

int ICACHE_FLASH_ATTR DHT_init (void)
{
    DHT_read_sem = xSemaphoreCreateMutex ();
    if (DHT_read_sem == NULL) {
        ESP_ERR ("failed to create semaphore");
        return 0;
    }

    //Setup & start the read timer
    os_timer_disarm (&DHT_read_timer);
    os_timer_setfn (&DHT_read_timer, (os_timer_func_t *)DHT_read_timeout, NULL);
    os_timer_arm (&DHT_read_timer, DHT_READ_INTERVAL, 1);

    return 1;
}

int DHT_get_temp ()
{
    int temp = 0;
    xSemaphoreTake (DHT_read_sem, portMAX_DELAY);
    temp = DHT_temp;
    xSemaphoreGive (DHT_read_sem);
    return temp;
}

int DHT_get_hum ()
{
    int hum = 0;
    xSemaphoreTake (DHT_read_sem, portMAX_DELAY);
    hum = DHT_hum;
    xSemaphoreGive (DHT_read_sem);
    return hum;
}


/**/
static void ICACHE_FLASH_ATTR DHT_read_timeout (void *pvParameters)
{
    int counter = 0;
    int laststate = 1;
    int i = 0;
    int j = 0;
    int checksum = 0;
    //int bitidx = 0;
    //int bits[250];

    int data[100] = { 0 };

    //data[0] = data[1] = data[2] = data[3] = data[4] = 0;

    GPIO_OUTPUT_SET (2, 1);
    os_delay_us (50000);
    os_delay_us (50000);
    os_delay_us (50000);
    os_delay_us (50000);
    os_delay_us (50000);
    GPIO_OUTPUT_SET (2, 0);
    os_delay_us (20000);
    GPIO_OUTPUT_SET (2, 1);
    os_delay_us (40);
    GPIO_DIS_OUTPUT (2);
    PIN_PULLUP_EN (PERIPHS_IO_MUX_GPIO2_U);


    // wait for pin to drop?
    while (GPIO_INPUT_GET (2) == 1 && i<100000) {
        os_delay_us (1);
        i++;
    }

    if (i == 100000)
        return;

    // read data!

    for (i = 0; i < MAXTIMINGS; i++) {
        counter = 0;
        while (GPIO_INPUT_GET (2) == laststate) {
            counter++;
            os_delay_us (1);
            if (counter == 1000)
                break;
        }
        laststate = GPIO_INPUT_GET (2);
        if (counter == 1000) break;

        //bits[bitidx++] = counter;

        if ((i>3) && (i % 2 == 0)) {
            // shove each bit into the storage bytes
            data[j / 8] <<= 1;
            if (counter > BREAKTIME)
                data[j / 8] |= 1;
            j++;
        }
    }

    /*
    for (i=3; i<bitidx; i+=2) {
    os_printf("bit %d: %d\n", i-3, bits[i]);
    os_printf("bit %d: %d (%d)\n", i-2, bits[i+1], bits[i+1] > BREAKTIME);
    }
    os_printf("Data (%d): 0x%x 0x%x 0x%x 0x%x 0x%x\n", j, data[0], data[1], data[2], data[3], data[4]);
    */

    ESP_DBG ("RAW data: %02X %02X | %02X %02X | %02X\n",
             (unsigned char)data[0], (unsigned char)data[1],
             (unsigned char)data[2], (unsigned char)data[3],
             (unsigned char)data[4]);

    int temp_p, hum_p;
    if (j >= 39) {
        checksum = (data[0] + data[1] + data[2] + data[3]) & 0xFF;
        if (data[4] == checksum) {
            /* yay! checksum is valid */

            hum_p = data[0] * 256 + data[1];
            hum_p *= 10;

            temp_p = (data[2] & 0x7F) * 256 + data[3];
            temp_p *= 10;
            if (data[2] & 0x80)
                temp_p *= -1;

            xSemaphoreTake (DHT_read_sem, portMAX_DELAY);
            DHT_temp = temp_p;
            DHT_hum = hum_p;
            xSemaphoreGive (DHT_read_sem);

            ESP_DBG ("Temp =  %d *C, Hum = %d \%\n", (int)(temp_p), (int)(hum_p));
        }
    }
}

#endif