
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

static const char *TAG = "Environment Sensor";




/* -- Declaration --------------------------------------------------- */
esp_err_t start_server(void);
static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);


/* -- Events Handlers --------------------------------------------------- */

/** Event handler */
static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data){

}





/* -- MAIN --------------------------------------------------- */

void app_main(void) {
	//vTaskDelay(1000);
	
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
	init_config();
	//Coms
	init_coms();
	
	//Wait for connection
	ESP_LOGI(TAG, "Waiting for connection before continuing to init" );
	while(isConnected == 0){
		// ESP_LOGI(TAG, "." ); 
		vTaskDelay(10);
	};

    ESP_ERROR_CHECK(start_server());

	// Sensor
	ESP_LOGI(TAG, "Sensor Initialize");
	init_sensors();
// #ifdef ACTIVE_SHT31
	sht3x_init_sensor(I2CSensor_BUS, SHT3x_ADDR_1);
// #endif // ACTIVE_SHT31	
#ifdef ACTIVE_D7s
	d7s_init_sensor(I2CSensor_BUS, D7S_ADDRESS);
#endif // ACTIVE_SHT31
	
	ESP_LOGI(TAG, "MQTT Initialize");
	mqtt_init();
	
	ESP_LOGI(TAG, "Initializing done" );
}
