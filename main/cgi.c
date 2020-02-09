#include "cgi.h"
//Template code for the counter on the index page.
CgiStatus ICACHE_FLASH_ATTR tplCurrentConfig(HttpdConnData *connData, char *token, void **arg) {
	char buff[128];
	if (token==NULL) return HTTPD_CGI_DONE;

	if (strcmp(token, "f_version")==0) {
		strcpy(buff, config_data.f_version);
	}
	
	if (strcmp(token, "h_version")==0) {
		strcpy(buff, config_data.h_version);
	}
	
	if (strcmp(token, "location")==0) {
		strcpy(buff, config_data.location);
	}
	httpdSend(connData, buff, -1);
	return HTTPD_CGI_DONE;
}