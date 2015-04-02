#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#define MYSSID        "myssid"
#define MYPASSPHRASE  "password"

#define U_CONFIG_SSID_MAX           32
#define U_CONFIG_WIFIPASS_MAX       64

/* Configuration parameters header structure */
struct u_config_header {
    unsigned char version;
    unsigned char length;
};

/* Configuration parameters main structure */
struct u_config {
    unsigned char version;
    unsigned char length;

    char ssid[U_CONFIG_SSID_MAX];
    char wifi_pass[U_CONFIG_WIFIPASS_MAX];
};



#endif

