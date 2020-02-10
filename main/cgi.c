#include "cgi.h"
#include "esp_log.h"
//Template code for the counter on the index page.
CgiStatus ICACHE_FLASH_ATTR tplCurrentConfig(HttpdConnData *connData, char *token, void **arg) {
	char buff[128];
	if (token==NULL) return HTTPD_CGI_DONE;
	if (strcmp(token, "F_VERSION")==0) {
		strcpy(buff, config_data.f_version);
	}
	
	if (strcmp(token, "H_VERSION")==0) {
		strcpy(buff, config_data.h_version);
	}
	
	if (strcmp(token, "S_LOCATION")==0) {
		strcpy(buff, config_data.location);
	}
	httpdSend(connData, buff, -1);
	return HTTPD_CGI_DONE;
}