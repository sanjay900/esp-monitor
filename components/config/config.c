#include "config.h"
#include "esp_log.h"
#include "esp_err.h"
#include <string.h>
#include "nvs_flash.h"
static const char *TAG = "Environment Sensor - Config Handler";
config_struct config_data={
	.f_version="1.3",
	.h_version="1.0",
	.id=12,
	.name="test_sensor",
	.location="it_room",
	.sensors={"TH","SI"},
	.sensor_count=2,
	.refresh_rate=1000,
	.mqtt_ip="mqtt://toxidy.forge.wetaworkshop.co.nz:1883"
};
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

static esp_err_t load_config(void){
	nvs_handle_t my_handle;
    esp_err_t err = nvs_open("config", NVS_READWRITE, &my_handle);
	size_t size = sizeof(config_data.f_version);
	err = nvs_get_str(my_handle, "f_version", config_data.f_version, &size);	
	size = sizeof(config_data.h_version);
	err = nvs_get_str(my_handle, "h_version", config_data.h_version, &size);	
	err = nvs_get_i32(my_handle, "id", &config_data.id);	
	size = sizeof(config_data.name);
	err = nvs_get_str(my_handle, "name", config_data.name, &size);	
	size = sizeof(config_data.location);
	err = nvs_get_str(my_handle, "location", config_data.location, &size);
	size = sizeof(config_data.sensors);	
	char sensors[MAX_SENSOR_COUNT*5];	
	err = nvs_get_str(my_handle, "sensors", sensors, &size);
	if (err == ESP_OK) {	
		config_data.sensor_count = 0;
		char *token;
		token = strtok(sensors, ",");
		while (token != NULL) {
			ESP_LOGI(TAG, "Reading: %d -> %s", config_data.sensor_count, token);
			strcpy(config_data.sensors[config_data.sensor_count++], token);
			token = strtok(NULL, ",");
		}
	}
	err = nvs_get_i32(my_handle, "refresh_rate", &config_data.refresh_rate);	
	size = sizeof(config_data.mqtt_ip);	
	err = nvs_get_str(my_handle, "mqtt_ip", config_data.mqtt_ip, &size);	
	nvs_close(my_handle);
	//ESP_LOGI(TAG, "%s", config_data);
	
	return ESP_OK;
}
void init_config(void) {
	// //Spiffs 
    // ESP_ERROR_CHECK(init_spiffs());
	//NVS
	ESP_ERROR_CHECK(init_NVS());
	//Config
	load_config();
}