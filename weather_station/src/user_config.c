/******************************************************************************
* Copyright 2015 liv
*
* FileName: user_config.c
*
* Description: stores, loads configuration from flash
*
*******************************************************************************/
#ifdef WIN32
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#else
#include "esp_common.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#endif
#include "debug.h"
#include "user_config.h"

//struct u_config esp_config = { 0 };

#ifdef WIN32
#else
xSemaphoreHandle u_config_sem = NULL;
#endif

#ifdef UC_VAR
#undef UC_VAR
#endif
#define UC_VAR(type, name, save, def)       \
    struct u_config_var_##type u_config_var_##name = {#name, save, def};
UC_DECALRE_ALL_VARS;

struct u_config_var_string u_config_var_wifi_ssid222 = { "wifi_ssid", 1, "myssid" }

void user_config_load (void)
{
    // load file/ flas and update all vars 
}

void user_config_save (void)
{
    // parse UC_DECALRE_ALL_VARS, serialize all vars with save flag set, and write to file/flash
}


void user_config_init (void)
{
    U_CONFIG_SEM_INIT ();
    user_config_load ();
}


