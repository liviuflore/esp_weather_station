#ifndef __WEBSITE_H__
#define __WEBSITE_H__

void www_init_webpages(void);
int www_build_response_from_uri(char* uri, char* response);
int www_get_response_from_uri(char* uri, char** response);

#endif /* __WEBSITE_H__ */
