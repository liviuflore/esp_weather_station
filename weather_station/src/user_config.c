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
#endif
#include "debug.h"
#include "user_config.h"

struct u_config esp_config = { 0 };


void user_config_load (void)
{

}

void user_config_save (void)
{

}
