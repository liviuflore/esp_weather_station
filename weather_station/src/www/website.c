#ifdef WIN32
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#else
#include "esp_common.h"
#endif
#include "debug.h"
#include "website.h"
#include "webpages.h"


const static char www_html_hdr[] = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n";
const static char www_css_hdr[]  = "HTTP/1.1 200 OK\r\nContent-type: text/css\r\n\r\n";
const static char www_png_hdr[]  = "HTTP/1.1 200 OK\r\nContent-type: image/png\r\n\r\n";

const static char www_404_html[] = "<html><head><title>404</title></head><body><h1>404</h1><p>Page not found</p></body></html>";

int www_get_404_page (char* response)
{
    memcpy (response, www_html_hdr, sizeof (www_html_hdr));
    memcpy (response + sizeof (www_html_hdr), www_404_html, sizeof (www_404_html));
    response[sizeof (www_html_hdr) + sizeof (www_404_html)] = '\0';
    return (sizeof (www_html_hdr) + sizeof (www_404_html));
}


int www_build_response_from_uri(char* uri, char* response)
{
    char* url = NULL;
    char* wp_content = NULL;
    char* wp_header = NULL;

    if (response == NULL) {
        ESP_ERR ("invalid input params");
        return 0;
    }

    /* get default page */
    if (!strncmp (uri, "/", 1) && strlen (uri) == 1)
        url = "/index.html";
    else
        url = uri;

    /* get header based on content type */
    if (strncmp (url + strlen (url) - 4, ".css", 4) == 0) {
        wp_header = (char*)www_css_hdr;
    }
    else if (strncmp (url + strlen (url) - 4, ".png", 4) == 0) {
        wp_header = (char*)www_png_hdr;
    }
    else {
        wp_header = (char*)www_html_hdr;
    }

    int offset = 0;
    int len = 0;

    len = strlen (wp_header);
    memcpy (response + offset, wp_header, len);
    offset += len;

    if (!strncmp (url + strlen (url) - 4, ".var", 4)) {
        //wp = www_variable_get (url);
        memcpy (response + offset, "N/A", sizeof ("N/A"));
        offset += sizeof ("N/A");
    }
    else {
        struct www_webpage* wp = www_webpages_get (url);

        if (wp == NULL) {
            memcpy (response + offset, www_404_html, sizeof (www_404_html));
            offset += sizeof (www_404_html);
        }

        if (wp->action != NULL) {
            offset += wp->action (wp->page, wp->size, response + offset);
        }
        else {
            len = strlen (wp->page);
            memcpy (response + offset, wp->page, len);
            offset += len;
        }
    }
    response[offset] = '\0';

    return offset;
}

#define WWW_MAX_VAR_NAME
struct www_variable {
    char name[32];

};

#define TOKENSTART      "<!--#VAR:"
#define TOKENEND        "#-->"
#define TOKENNAMELENGTH 32

int www_replace_token (char* in, char *out, char* token, char* val, int *out_size)
{
    char    curr_token[TOKENNAMELENGTH] = { 0 };
    int     curr_token_len = 0;
    char    *tmp = NULL;
    char    *replace_start = NULL;
    char    *replace_end = NULL;
    int     token_start_size = strlen (TOKENSTART);
    int     token_end_size = strlen (TOKENEND);
    int     found = 0;

    if (in == NULL || out == NULL || token == NULL || val == NULL || out_size == NULL) {
        ESP_ERR ("invalid input params");
        return -1;
    }

    /* Find 1st Token start */
    tmp = in;
    while (1) {
        /* find token start */
        tmp = strstr (tmp, TOKENSTART);
        if (tmp == NULL)
            break;
        replace_start = tmp;
        tmp += token_start_size;

        /* find token end */
        tmp = strstr (tmp, TOKENEND);
        if (tmp == NULL)
            break;
        tmp += token_end_size;
        replace_end = tmp;

        /* check token len */
        curr_token_len = replace_end - replace_start - token_start_size - token_end_size;
        if (curr_token_len >= TOKENNAMELENGTH) {
            ESP_ERR ("token length is too high (max supporte val %d, val %d)", TOKENNAMELENGTH - 1, curr_token_len);
            break;
        }

        /* get current token name */
        memcpy (curr_token, replace_start + token_start_size, curr_token_len);
        curr_token[curr_token_len] = '\0';

        /* check if correct token */
        if (!strncmp (curr_token, token, strlen (token))) {
            found = 1;
            break;
        }
    }

    if (found) {
        /* token found, replace it with the value */
        /* TODO: alloc new buff because in ptr could be equal to out ptr */
        int pt1_size = replace_start - in;
        int val_size = strlen (val);
        int pt2_size = strlen (in) - (replace_end - in);

        char* out1_ptr = (char *)malloc (pt1_size + 1);
        char* out2_ptr = (char *)malloc (pt2_size + 1);

        if (out1_ptr == NULL || out2_ptr == NULL) {
            ESP_ERR ("malloc failed");
            return -1;
        }

        /* copy first part */
        memcpy (out1_ptr, in, pt1_size); 
        out1_ptr[pt1_size] = '\0';

        /* copy second part */
        memcpy (out2_ptr, replace_end, pt2_size);
        out2_ptr[pt2_size] = '\0';

        /* compose output */
        *out_size = sprintf (out, "%s%s%s", out1_ptr, val, out2_ptr);

        free (out1_ptr);
        free (out2_ptr);

        return 0;
    }
    else {
        /* token not found, should leave the page as it is */
        int in_size = strlen (in);
        memcpy (out, in, in_size);
        out[in_size] = '\0';

        *out_size = in_size;

        return 0;
    }
}

int www_rsp_index_html (char* in, int size, char *out)
{
    int out_size = 0;
    int ret = 0;

    ret |= www_replace_token (in, out, "TEST_VARIABLE", "MYHTTPD", &out_size);

    if (ret)
        return 0;
    return out_size;
}


void www_init_webpages(void)
{
    www_webpages_init();

    www_webpages_register_action ("/index.html", &www_rsp_index_html);
}


