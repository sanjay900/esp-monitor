
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"

#include "esp_netif.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_eth.h"
#include "esp_wifi.h"
#include "net_handler.h"
#include "config.h"
#include "mqtt.h"
#include "driver/i2c.h"
#include "sensor.h"
#include "sht3x.h"
#include "d7s.h"
#include <libesphttpd/esp.h>
#include "libesphttpd/httpd.h"
#include "espfs.h"
#include "espfs_image.h"
#include "libesphttpd/httpd-espfs.h"
#include "libesphttpd/cgiwifi.h"
#include "libesphttpd/cgiflash.h"
#include "libesphttpd/auth.h"
#include "libesphttpd/captdns.h"
#include "libesphttpd/cgiwebsocket.h"
#include "libesphttpd/httpd-freertos.h"
#include "libesphttpd/route.h"

#define I2CSensor_BUS 0
#define I2CSensor_SCL_PIN 5
#define I2CSensor_SDA_PIN 33
#define I2CSensor_FREQ I2C_FREQ_400K
#define LISTEN_PORT     80u
#define MAX_CONNECTIONS 32u

static char connectionMemory[sizeof(RtosConnType) * MAX_CONNECTIONS];
static HttpdFreertosInstance httpdFreertosInstance;

const HttpdBuiltInUrl builtInUrls[]={
	ROUTE_REDIRECT("/", "/index.html"),
	ROUTE_FILESYSTEM(),
	ROUTE_END()
};
static const char *TAG = "Environment Sensor";
/* -- MAIN --------------------------------------------------- */
void initialise_sensors(void) {
	ESP_LOGI(TAG, "Sensor Initialize, %d", config_data.sensor_count);
	// Init i2c
	i2c_init(I2CSensor_BUS, I2CSensor_SCL_PIN, I2CSensor_SDA_PIN, I2CSensor_FREQ);
	// Loop through sensors and initialise
	for (int i = 0; i < config_data.sensor_count; i++) {
		ESP_LOGI(TAG, "Sensor %s Init", config_data.sensors[i]);
		if (strcmp(config_data.sensors[i], "TH") == 0) {
			sht3x_init_sensor(I2CSensor_BUS, SHT3x_ADDR_1);
		}
		if (strcmp(config_data.sensors[i], "SI") == 0) {
			d7s_init_sensor(I2CSensor_BUS, D7S_ADDRESS);
		}
	}
	// Initialise sensor reading task 
	init_sensor_read_task();

}
void app_main(void) {
	ESP_LOGI(TAG, "Initializing processes" );
	ESP_LOGI(TAG, "IDF-Version" );
	ESP_LOGI(TAG, "%s" , IDF_VER);
	esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);
	// init_config();
	//Coms
	init_coms();
	
	//Wait for connection
	ESP_LOGI(TAG, "Waiting for connection before continuing to init" );
	while(isConnected == 0){
		// ESP_LOGI(TAG, "." ); 
		vTaskDelay(10);
	};
    EspFsConfig espfs_conf = {
		.memAddr = espfs_image_bin,
	};
	EspFs* fs = espFsInit(&espfs_conf);
    httpdRegisterEspfs(fs);
	httpdFreertosInit(&httpdFreertosInstance,
	                  builtInUrls,
	                  LISTEN_PORT,
	                  connectionMemory,
	                  MAX_CONNECTIONS,
	                  HTTPD_FLAG_NONE);
	httpdFreertosStart(&httpdFreertosInstance);
	// initialise_sensors();
	
	ESP_LOGI(TAG, "MQTT Initialize");
	// mqtt_init();
	
	ESP_LOGI(TAG, "Initializing done" );
}
