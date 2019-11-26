
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"

#include "nvs_flash.h"
#include "esp_spiffs.h"

#include "tcpip_adapter.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_eth.h"
#include "esp_wifi.h"
#include "mqtt_client.h"
#include "net_handler.h"
#include "config.h"

static const char *TAG = "Environment Sensor";

#define ACTIVE_SCREEN	// OLED1306
#define ACTIVE_WIFI		// WIFI
//#define ACTIVE_ETHERNET	// ETHERNET
#define ACTIVE_SHT31	// Temperature/humidity sensor
//#define ACTIVE_D7s		// Siesmic sensor

#ifdef ESP_PLATFORM // ESP32 (ESP-IDF)
	#define TASK_STACK_DEPTH 2048	// user task stack depth for ESP32
#else // ESP8266 (esp-open-rtos)
	#define TASK_STACK_DEPTH 256	// user task stack depth for ESP8266
#endif // ESP_PLATFORM


#ifdef ACTIVE_SHT31
	#include "sht3x.h"
	
	#define I2CSensor_BUS 0
	#define I2CSensor_SCL_PIN 22
	#define I2CSensor_SDA_PIN 21
	#define I2CSensor_FREQ I2C_FREQ_100K
	static sht3x_sensor_t *sensor_sht31; // sensor device data structure
	static float temperature=-1;
	static float humidity=-1;
#endif // ACTIVE_SHT31

#ifdef ACTIVE_SCREEN
	#include "ssd1306.h"
	#include "ssd1306_default_if.h"
	#include "ssd1306_draw.h"
	#include "ssd1306_font.h"

	#define USE_I2C_DISPLAY
	static const int I2CDisplayAddress = 0x3C;
	static const int I2CDisplayWidth = 128;
	static const int I2CDisplayHeight = 64;
	static const int I2CResetPin = 16;
	struct SSD1306_Device I2CDisplay;
#endif // ACTIVE_SCREEN

static esp_mqtt_client_handle_t mqttClient;

static bool mqttStatus = 0;
static bool isConnected = 0;
static char mqttTopic[30];
static char humTopic[30];
static char tempTopic[30];

/* -- Declaration --------------------------------------------------- */
esp_err_t start_server(void);
static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);



/** INIT MQTT **/
static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = config_data.mqtt_ip,
    };

    mqttClient = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqttClient, ESP_EVENT_ANY_ID, mqtt_event_handler, mqttClient);
    esp_mqtt_client_start(mqttClient);
	strcpy(mqttTopic, config_data.name);
	strcat(mqttTopic, "\\");
	strcat(mqttTopic, config_data.location);
	strcat(mqttTopic, "\\");
	strcat(mqttTopic, config_data.sensor_type);
	strcat(mqttTopic, "\\");
	strcpy(mqttTopic, tempTopic);
	strcat(tempTopic, "temperature");
	strcpy(mqttTopic, humTopic);
	strcat(humTopic, "humidity");
}

/* -- Events Handlers --------------------------------------------------- */

/** Event handler */
static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data){

}


/** Event handler for MQTT events */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;    
	// esp_mqtt_client_handle_t client = event->client;
    // int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            mqttStatus = 1;
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
			mqttStatus = 0;
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            ESP_LOGI(TAG, "TOPIC=%.*s", event->topic_len, event->topic);
            ESP_LOGI(TAG, "DATA=%.*s", event->data_len, event->data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            ESP_LOGI(TAG, "Other event id:%d", event->event_id);
            break;
    }
    //return ESP_OK;
}

#ifdef ACTIVE_SCREEN
/** Display TASK */
void display_task(void *pvParameters) {
	TickType_t last_wakeup = xTaskGetTickCount();
	while (1) {
		SSD1306_Clear(&I2CDisplay, SSD_COLOR_BLACK);
		char sensorData[36], ipAdd[18];
		tcpip_adapter_ip_info_t ipInfo;
#ifdef ACTIVE_SHT31
		sprintf(sensorData, "T:%3.2f%cC  H:%3.2f%%", temperature,(char)176,humidity);
#endif // ACTIVE_SHT31
#ifdef ACTIVE_WIFI
		tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo);
#endif // ACTIVE_WIFI
#ifdef ACTIVE_ETHERNET
		tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ipInfo);
#endif // ACTIVE_ETHERNET
		SSD1306_FontDrawString(&I2CDisplay, 0, 0, sensorData, SSD_COLOR_WHITE);
		//SSD1306_FontDrawString(&I2CDisplay, 0, I2CDisplayHeight/4, mqtt_status_str,SSD_COLOR_WHITE);
		//SSD1306_FontDrawString(&I2CDisplay, 0, 2*I2CDisplayHeight/4, config_data.location, SSD_COLOR_WHITE);
		sprintf(ipAdd,IPSTR, IP2STR(&ipInfo.ip));
		SSD1306_FontDrawString(&I2CDisplay, 0, 3*I2CDisplayHeight/4, ipAdd, SSD_COLOR_WHITE);
		SSD1306_Update(&I2CDisplay);
		// Wait until 2 seconds (cycle time) are over.
		vTaskDelayUntil(&last_wakeup, config_data.refresh_rate / portTICK_PERIOD_MS);
	}
}
#endif // ACTIVE_SCREEN

void sensor_read(void *pvParameters) {
#ifdef ACTIVE_SHT31
	// Start periodic measurements with 1 measurement per second.
	sht3x_start_measurement(sensor_sht31, sht3x_periodic_1mps, sht3x_high);

	// Wait until first measurement is ready (constant time of at least 30 ms
	// or the duration returned from *sht3x_get_measurement_duration*).
	vTaskDelay(sht3x_get_measurement_duration(sht3x_high));
#endif // ACTIVE_SHT31

	TickType_t last_wakeup = xTaskGetTickCount();

	while (1) {
#ifdef ACTIVE_SHT31
		// Get the values and do something with them.
		if (sht3x_get_results(sensor_sht31, &temperature, &humidity)){
			ESP_LOGI(TAG, "%.3f SHT3x Sensor: %.2f °C, %.2f %%\n",
					(double)(clock() * 1000 / CLOCKS_PER_SEC), temperature, humidity);
		} 
		else {
			ESP_LOGE(TAG, "sht31_readTempHum : failed");
		}
#endif // ACTIVE_SHT31
		// Wait until 2 seconds (cycle time) are over.
		vTaskDelayUntil(&last_wakeup, config_data.refresh_rate / portTICK_PERIOD_MS);
	}
}

void mqtt_send(void *pvParameters) {
	char temperatureChar[64], humidityChar[64];
	int msg_id;
	TickType_t last_wakeup = xTaskGetTickCount();

	while (1) {
		if(mqttStatus){
			sprintf(temperatureChar, "%f", temperature);
			sprintf(humidityChar, "%f", humidity);
			msg_id = esp_mqtt_client_publish(mqttClient, tempTopic, temperatureChar, 0, 1, 0);
			ESP_LOGD(TAG, "sent publish successful, msg_id=%d", msg_id);
			msg_id = esp_mqtt_client_publish(mqttClient, humTopic, humidityChar, 0, 1, 0);
			ESP_LOGD(TAG, "sent publish successful, msg_id=%d", msg_id);
		}
		else{
			//reconnect
			while(esp_mqtt_client_reconnect(mqttClient) != ESP_OK);
		}
		
		// Wait until 2 seconds (cycle time) are over.
		vTaskDelayUntil(&last_wakeup, config_data.refresh_rate / portTICK_PERIOD_MS);
	}

}

/* -- MAIN --------------------------------------------------- */

void app_main(void) {
	//vTaskDelay(1000);
	
	ESP_LOGD(TAG, "Initializing processes" );
	ESP_LOGD(TAG, "IDF-Version" );
	ESP_LOGD(TAG, "%s" , IDF_VER);
	/*esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);*/
	init_config();
	//Coms
	init_coms();
	
#ifdef ACTIVE_SCREEN
	// Init Screen
	ESP_LOGD(TAG, "Screen Initialize" );
	if( SSD1306_I2CMasterInitDefault( ) == true ){
		if( SSD1306_I2CMasterAttachDisplayDefault( &I2CDisplay, I2CDisplayWidth, I2CDisplayHeight, I2CDisplayAddress, I2CResetPin ) == true ){
			SSD1306_Clear(&I2CDisplay, SSD_COLOR_BLACK);
			SSD1306_SetFont(&I2CDisplay, &Font_droid_sans_fallback_11x13);
			ESP_LOGD(TAG, "Screen Init OK" );
			SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_NorthWest, "Waiting for connection...", SSD_COLOR_WHITE );
			SSD1306_Update( &I2CDisplay );
		}
		else{
			ESP_LOGE(TAG, "Screen attach failed" );
		}
	}
	else{
		ESP_LOGE(TAG, "Screen I2C init failed" );
	}
#endif // ACTIVE_SCREEN	
	
	//Wait for connection
	ESP_LOGI(TAG, "Waiting for connection before continuing to init" );
	while(isConnected == 0){
		ESP_LOGI(TAG, "." );
	};

    ESP_ERROR_CHECK(start_server());

		
#ifdef ACTIVE_SCREEN
	// Create a user task that uses the display, low priority.
	xTaskCreate(display_task, "display_task", TASK_STACK_DEPTH, NULL, 1, 0);
#endif // ACTIVE_SCREEN	

	// Sensor
	ESP_LOGD(TAG, "Sensor Initialize");
	// Create the sensor
	esp_err_t sensor = ESP_FAIL;
#ifdef ACTIVE_SHT31
	i2c_init(I2CSensor_BUS, I2CSensor_SCL_PIN, I2CSensor_SDA_PIN, I2CSensor_FREQ);
	sensor_sht31 = sht3x_init_sensor(I2CSensor_BUS, SHT3x_ADDR_1);
	sensor = (esp_err_t)sensor_sht31;
#endif // ACTIVE_SHT31	
	
	if (sensor) {
		ESP_LOGD(TAG, "Sensor Init OK" );
		// Create a user task that uses the sensors.
		xTaskCreatePinnedToCore(sensor_read, "sensor_read", TASK_STACK_DEPTH, NULL, 2,  NULL, 0);	//xTaskCreatePinnedToCore
	}
	else{
		ESP_LOGE(TAG, "Sensor Init Failed" );
	}
	
	
	ESP_LOGD(TAG, "MQTT Initialize");
	mqtt_app_start();
	xTaskCreatePinnedToCore(mqtt_send, "mqtt_send", TASK_STACK_DEPTH, NULL, 1, NULL, 1);	//xTaskCreatePinnedToCore
	
	ESP_LOGD(TAG, "Initializing done" );
}
