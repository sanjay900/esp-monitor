#include "cgi.h"
#include "esp_log.h"
#include "nvs_flash.h"
// Template code for the counter on the index page.
CgiStatus ICACHE_FLASH_ATTR tplCurrentConfig(HttpdConnData *connData,
                                             char *token, void **arg) {
  char buff[128];
  if (token == NULL)
    return HTTPD_CGI_DONE;
  if (strcmp(token, "F_VERSION") == 0) {
    strcpy(buff, config_data.f_version);
  }

  if (strcmp(token, "H_VERSION") == 0) {
    strcpy(buff, config_data.h_version);
  }

  if (strcmp(token, "S_LOCATION") == 0) {
    strcpy(buff, config_data.location);
  }

  if (strcmp(token, "S_NAME") == 0) {
    strcpy(buff, config_data.name);
  }

  if (strcmp(token, "S_ID") == 0) {
    sprintf(buff, "%d", config_data.id);
  }

  if (strcmp(token, "S_RATE") == 0) {
    sprintf(buff, "%d", config_data.refresh_rate);
  }

  if (strcmp(token, "S_MQTT") == 0) {
    strcpy(buff, config_data.mqtt_ip);
  }

  if (strcmp(token, "S_SENSORS") == 0) {
    size_t len = 0;
    for (int i = 0; i < config_data.sensor_count; i++) {
	    len += sprintf(buff+len, "%s,", config_data.sensors[i]);
    }
  }
  httpdSend(connData, buff, -1);
  return HTTPD_CGI_DONE;
}
HttpdPlatTimerHandle httpdPlatTimerCreate(const char *name, int periodMs, int autoreload, void (*callback)(void *arg), void *ctx);
void httpdPlatTimerStart(HttpdPlatTimerHandle timer);
void httpdPlatTimerStop(HttpdPlatTimerHandle timer);
void httpdPlatTimerDelete(HttpdPlatTimerHandle timer);
static HttpdPlatTimerHandle resetTimer;

static void ICACHE_FLASH_ATTR resetTimerCb(void *arg) {
  esp_restart();
}
CgiStatus ICACHE_FLASH_ATTR cgiConfig(HttpdConnData *connData) {
	int len;
	char buff[1024];

	if (connData->isConnectionClosed) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}
  
	nvs_handle_t my_handle;
  nvs_open("config", NVS_READWRITE, &my_handle);
	len=httpdFindArg(connData->post.buff, "name", buff, sizeof(buff));
	if (len!=0) {
    nvs_set_str(my_handle, "name", buff);
	}

	len=httpdFindArg(connData->post.buff, "loc", buff, sizeof(buff));
	if (len!=0) {
    nvs_set_str(my_handle, "location", buff);
	}

	len=httpdFindArg(connData->post.buff, "id", buff, sizeof(buff));
	if (len!=0) {
    nvs_set_i32(my_handle, "id", atoi(buff));
	}

	len=httpdFindArg(connData->post.buff, "rate", buff, sizeof(buff));
	if (len!=0) {
    nvs_set_i32(my_handle, "refresh_rate", atoi(buff));
	}

	len=httpdFindArg(connData->post.buff, "mqtt", buff, sizeof(buff));
	if (len!=0) {
    nvs_set_str(my_handle, "mqtt_ip", buff);
	}
  len = 0;
  len=httpdFindArg(connData->post.buff, "TH", buff, sizeof(buff));
	if (len!=0) {
    buff[len-1] = ',';
    buff[len] = '\0';
	}
  if (len == -1) len = 0;
  len+=httpdFindArg(connData->post.buff, "SI", buff+len, sizeof(buff-len));
	if (len!=0) {
    nvs_set_str(my_handle, "sensors", buff);
	}
  
	ESP_LOGI("CGI", "%s", buff);
  nvs_commit(my_handle);
  nvs_close(my_handle);
	httpdRedirect(connData, "index.tpl");
  resetTimer=httpdPlatTimerCreate("flashreset", 500, 0, resetTimerCb, NULL);
	httpdPlatTimerStart(resetTimer);
	return HTTPD_CGI_DONE;
}