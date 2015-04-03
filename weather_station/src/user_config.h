#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#include "debug.h"

#define MYSSID        "myssid"
#define MYPASSPHRASE  "password"




#define U_CONFIG_VAR_NAME_MAX       32
#define U_CONFIG_VAR_VALUE_MAX      64

enum u_config_type {
    UC_INT,
    UC_STRING,
};

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


#define UC_DECALRE_ALL_VARS                                         \
    UC_VAR (int, temperature, 0, 0)                                 \
    UC_VAR (int, humidity, 0, 0)                                    \
    UC_VAR (string, wifi_ssid, 1, "")                               \
    UC_VAR (string, wifi_pass, 1, "")


#ifdef UC_VAR
#undef UC_VAR
#endif
#define UC_VAR(type, name, save, def)                               \
    extern struct u_config_var_##type u_config_var_##name;
UC_DECALRE_ALL_VARS;


#define UC_GET_VAR(name) u_config_var_##name.value
#define UC_SET_VAR(name, val) u_config_var_##name.value = val

#define UC_SET_VAR_STR(name, val)                                   \
    {                                                               \
        int len = strlen(val);                                      \
        ESP_ASSERT (len < U_CONFIG_VAR_VALUE_MAX);                  \
        memcpy (u_config_var_##name.value, val, strlen(val));       \
    }


#endif


