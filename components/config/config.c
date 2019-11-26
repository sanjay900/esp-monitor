#include "config.h"
#include "esp_log.h"
#include "esp_err.h"
#include <string.h>
#include "nvs_flash.h"
#include "esp_spiffs.h"   
static const char *TAG = "Environment Sensor - Config Handler";
/** INIT NVS **/
esp_err_t init_NVS(void){
    ESP_LOGD(TAG, "Initializing NVS");
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
    }
    return ret;
}

/** INIT SPIFFS **/
esp_err_t init_spiffs(void){
    ESP_LOGD(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
    .base_path = "/spiffs",
    .partition_label = NULL,
    .max_files = 10,   // This decides the maximum number of files that can be created on the storage
    .format_if_mount_failed = true
    };

    esp_err_t ret = esp_vfs_spiffs_register(&conf);
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ESP_FAIL;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
        return ESP_FAIL;
    }

    ESP_LOGD(TAG, "Partition size: total: %d, used: %d", total, used);
    return ESP_OK;
}


/* -- User tasks --------------------------------------------------- */
char *extract_between(const char *str, const char *p1, const char *p2){
	const char *i1 = strstr(str, p1);
	if(i1 != NULL){
		const size_t pl1 = strlen(p1);
		const char *i2 = strstr(i1 + pl1, p2);
		if(p2 != NULL){
			/* Found both markers, extract text. */
			const size_t mlen = i2 - (i1 + pl1);
			char *ret = malloc(mlen + 1);
			if(ret != NULL){
				memcpy(ret, i1 + pl1, mlen);
				ret[mlen] = '\0';
				return ret;
			}
		}
	}
	return NULL;
}
  
static esp_err_t load_config(void){
	const char *filepath = "/spiffs/config.txt";
    FILE *fd = NULL;

    fd = fopen(filepath, "r");
    if (!fd) {
        ESP_LOGE(TAG, "Failed to open file : %s", filepath);
        return ESP_FAIL;
    }
	fseek(fd, 0, SEEK_END);
	unsigned int file_size = ftell(fd);
	fseek(fd, 0, SEEK_SET);
	char *file_buf = (char *)malloc(file_size);
	fread(file_buf, file_size, 1, fd);
	
    fclose(fd);
	ESP_LOGI(TAG, "Config file content : ");
	ESP_LOGI(TAG, "%s", file_buf);
	
	strcpy(config_data.f_version, extract_between(file_buf, "f_version=", ";"));
	strcpy(config_data.h_version, extract_between(file_buf, "h_version=", ";"));
	config_data.id = atoi(extract_between(file_buf, "id=", ";"));
	strcpy(config_data.name, extract_between(file_buf, "name=", ";"));
	strcpy(config_data.location, extract_between(file_buf, "location=", ";"));
	strcpy(config_data.sensor_type, extract_between(file_buf, "sensor_type=", ";"));
	config_data.refresh_rate = atoi(extract_between(file_buf, "refresh_rate=", ";"));
	config_data.alarm_treshold_min = atoi(extract_between(file_buf, "alarm_treshold_min=", ";"));
	config_data.alarm_treshold_max = atoi(extract_between(file_buf, "alarm_treshold_max=", ";"));
	strcpy(config_data.esp32_ip, extract_between(file_buf, "esp32_ip=", ";"));
	strcpy(config_data.mqtt_ip, extract_between(file_buf, "mqtt_ip=", ";"));
	config_data.log_activ = atoi(extract_between(file_buf, "log_activ=", ";"));
	config_data.log_days = atoi(extract_between(file_buf, "log_days=", ";"));

	//ESP_LOGI(TAG, "%s", config_data);
	
	return ESP_OK;
}
void init_config(void) {
	//Spiffs 
    ESP_ERROR_CHECK(init_spiffs());
	//Config
	load_config();
	//NVS
	ESP_ERROR_CHECK(init_NVS());
}