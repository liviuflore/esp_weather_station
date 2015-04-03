#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#ifdef WIN32
#else
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#endif

#include "debug.h"

#define MYSSID        "myssid"
#define MYPASSPHRASE  "password"



/* semaphore for config access */
#ifdef WIN32
#define U_CONFIG_SEM_INIT()
#define U_CONFIG_SEM_TAKE()
#define U_CONFIG_SEM_GIVE()
#else
extern xSemaphoreHandle u_config_sem;
#define U_CONFIG_SEM_INIT()                                     \
        u_config_sem = xSemaphoreCreateMutex ();                \
        ESP_ASSERT (u_config_sem != NULL);
#define U_CONFIG_SEM_TAKE()                                     \
        xSemaphoreTake (u_config_sem, portMAX_DELAY);
#define U_CONFIG_SEM_GIVE()                                     \
        xSemaphoreGive (u_config_sem);

#endif


/* configuration declaration */

#define U_CONFIG_VAR_VALUE_MAX      64

#define UC_DECALRE_ALL_VARS                                     \
    UC_VAR (int,    temperature,            0,  0)              \
    UC_VAR (int,    humidity,               0,  0)              \
    UC_VAR (int,    update_interval,        0, 60)              \
    UC_VAR (string, wifi_ssid,              1, "")              \
    UC_VAR (string, wifi_pass,              1, "")



struct u_config_var {
    char* name;
    unsigned char save;
};

struct u_config_var_int {
    char* name;
    unsigned char save;
    int value;
};
struct u_config_var_string {
    char* name;
    unsigned char save;
    char value[U_CONFIG_VAR_VALUE_MAX];
};

#ifdef UC_VAR
#undef UC_VAR
#endif
#define UC_VAR(type, name, save, def)                               \
    extern struct u_config_var_##type u_config_var_##name;
UC_DECALRE_ALL_VARS;

/* configuration get / set */

#define UC_GET_VAR(name, val)                                       \
    {                                                               \
        U_CONFIG_SEM_TAKE ();                                       \
        val = u_config_var_##name.value;                            \
        U_CONFIG_SEM_GIVE();                                        \
    }

#define UC_GET_VAR2(name)    u_config_var_##name.value

#define UC_GET_VAR_FROM_STRING(str_name)    user_config_get_var_from_string(str_name);


#define UC_SET_VAR(name, val)                                       \
    {                                                               \
        U_CONFIG_SEM_TAKE ();                                       \
        u_config_var_##name.value = val;                            \
        U_CONFIG_SEM_GIVE();                                        \
    }

#define UC_SET_VAR_STR(name, val)                                   \
    {                                                               \
        int len = strlen(val);                                      \
        ESP_ASSERT (len < U_CONFIG_VAR_VALUE_MAX);                  \
        U_CONFIG_SEM_TAKE ();                                       \
        memcpy (u_config_var_##name.value, val, strlen(val));       \
        U_CONFIG_SEM_GIVE();                                        \
    }


#endif


