
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_spiffs.h"
#include "tcpip_adapter.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "mqtt_handler.h"
#include "sht3x.h"
#include "ssd1306.h"
#include "ssd1306_default_if.h"
#include "ssd1306_draw.h"
#include "ssd1306_font.h"

static const char *TAG = "Environment Sensor";

#ifdef ESP_PLATFORM // ESP32 (ESP-IDF)
	#define TASK_STACK_DEPTH 2048	// user task stack depth for ESP32
#else // ESP8266 (esp-open-rtos)
	#define TASK_STACK_DEPTH 256	// user task stack depth for ESP8266
#endif // ESP_PLATFORM

#define ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
#define WIFI_MAXIMUM_RETRY 5
/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;
/* The event group allows multiple bits for each event, but we only care about one event 
 * - are we connected to the AP with an IP? */
const int WIFI_CONNECTED_BIT = BIT0;
static int s_retry_num = 0;
static char ipAdress[] = "000.000.000.000";

#define I2CSensor_BUS 0
#define I2CSensor_SCL_PIN 22
#define I2CSensor_SDA_PIN 21
#define I2CSensor_FREQ I2C_FREQ_100K
static sht3x_sensor_t *sensor; // sensor device data structure
static float temperature=-1;
static float humidity=-1;

#define USE_I2C_DISPLAY
static const int I2CDisplayAddress = 0x3C;
static const int I2CDisplayWidth = 128;
static const int I2CDisplayHeight = 64;
static const int I2CResetPin = 16;
struct SSD1306_Device I2CDisplay;

/* -- user tasks --------------------------------------------------- */

static void event_handler(void* arg, esp_event_base_t event_base, 
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
		strcpy(ipAdress,"000.000.000.000");
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < WIFI_MAXIMUM_RETRY) {
			strcpy(ipAdress,"000.000.000.000");
            esp_wifi_connect();
            xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
		sprintf(ipAdress, "%s", ip4addr_ntoa(&event->ip_info.ip));
        ESP_LOGI(TAG, "got ip:%s", ipAdress);
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event) {
	char* msg = "";
	// your_context_t *context = event->context;
	switch (event->event_id) {
		case MQTT_EVENT_CONNECTED:
			msg = "MQTT Connected";
			break;
		case MQTT_EVENT_DISCONNECTED:
			msg = "MQTT Disconnected";
			break;
		default:
			break;
	}
	SSD1306_FontDrawAnchoredString(&I2CDisplay, TextAnchor_North, msg,SSD_COLOR_WHITE);
	SSD1306_Update(&I2CDisplay);
	return ESP_OK;
}

void init_coms(void){
	//Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
	
	//Init WIFI
    s_wifi_event_group = xEventGroupCreate();

    tcpip_adapter_init();

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = ESP_WIFI_SSID,
            .password = ESP_WIFI_PASS
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");
    ESP_LOGI(TAG, "connect to ap SSID:%s password:%s",
             ESP_WIFI_SSID, ESP_WIFI_PASS);
}

/* Function to initialize SPIFFS */
static esp_err_t init_spiffs(void){
    ESP_LOGI(TAG, "Initializing SPIFFS");

    esp_vfs_spiffs_conf_t conf = {
      .base_path = "/spiffs",
      .partition_label = NULL,
      .max_files = 5,   // This decides the maximum number of files that can be created on the storage
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

    ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    return ESP_OK;
}

/* Declare the function which starts the file server.
 * Implementation of this function is to be found in
 * server.c */
esp_err_t start_server(void);

bool init_screen(void) {
	assert( SSD1306_I2CMasterInitDefault( ) == true );
	assert( SSD1306_I2CMasterAttachDisplayDefault( &I2CDisplay, I2CDisplayWidth, I2CDisplayHeight, 												I2CDisplayAddress, I2CResetPin ) == true );
	SSD1306_Clear(&I2CDisplay, SSD_COLOR_BLACK);
	SSD1306_SetFont(&I2CDisplay, &Font_droid_sans_fallback_11x13);
	return true;
}

void display_task(void *pvParameters) {
	TickType_t last_wakeup = xTaskGetTickCount();
	while (1) {
		SSD1306_Clear(&I2CDisplay, SSD_COLOR_BLACK);
		char sensorData[36];
		sprintf(sensorData, "T:%3.2f%cC  H:%3.2f%%", temperature,(char)176,humidity);
		SSD1306_FontDrawAnchoredString(&I2CDisplay, TextAnchor_NorthWest, sensorData, SSD_COLOR_WHITE);
		
		SSD1306_FontDrawAnchoredString(&I2CDisplay, TextAnchor_SouthWest, ipAdress, SSD_COLOR_WHITE);
		SSD1306_Update(&I2CDisplay);
		// Wait until 2 seconds (cycle time) are over.
		vTaskDelayUntil(&last_wakeup, 2000 / portTICK_PERIOD_MS);
	}
}

void sensor_read(void *pvParameters) {

	// Start periodic measurements with 1 measurement per second.
	sht3x_start_measurement(sensor, sht3x_periodic_1mps, sht3x_high);

	// Wait until first measurement is ready (constant time of at least 30 ms
	// or the duration returned from *sht3x_get_measurement_duration*).
	vTaskDelay(sht3x_get_measurement_duration(sht3x_high));

	TickType_t last_wakeup = xTaskGetTickCount();

	while (1) {
		// Get the values and do something with them.
		if (sht3x_get_results(sensor, &temperature, &humidity)){
			ESP_LOGI(TAG, "%.3f SHT3x Sensor: %.2f °C, %.2f %%\n",
					(double)(clock() * 1000 / CLOCKS_PER_SEC), temperature, humidity);
		} 
		else {
			ESP_LOGI(TAG, "sht31_readTempHum : failed");
		}
		// Wait until 2 seconds (cycle time) are over.
		vTaskDelayUntil(&last_wakeup, 2000 / portTICK_PERIOD_MS);
	}
}

void app_main(void) {
	vTaskDelay(1000);
	
	//Coms
	ESP_LOGD(TAG, "Coms Initialize" );
	init_coms();
	
	/* Start spiffs */
    init_spiffs();

    ESP_ERROR_CHECK(start_server());
	
	//Screen
	ESP_LOGD(TAG, "Screen Initialize" );
	if ( init_screen() == true ) {
        ESP_LOGD(TAG, "Screen Init OK" );
		SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_NorthWest, "MQTT Disconnected", SSD_COLOR_WHITE );
		SSD1306_Update( &I2CDisplay );
		// Create a user task that uses the display, low priority.
		xTaskCreate(display_task, "display_task", TASK_STACK_DEPTH, NULL, 1, 0);
    }
	else{
		ESP_LOGE(TAG, "Screen Init Failed" );
	}
	
	//ESP_LOGD(TAG, "MQTT Initialize");
	//init_mqtt(mqtt_event_handler, "mqtt://172.22.2.163:1883");

	// Sensor
	ESP_LOGD(TAG, "SHT31 Initialize");
	// Create the sensor
	i2c_init(I2CSensor_BUS, I2CSensor_SCL_PIN, I2CSensor_SDA_PIN, I2CSensor_FREQ);
	if ((sensor = sht3x_init_sensor(I2CSensor_BUS, SHT3x_ADDR_1))) {
		ESP_LOGD(TAG, "SHT31 Sensor Init OK" );
		// Create a user task that uses the sensors.
		//xTaskCreate(sensor_read, "sensor_read", TASK_STACK_DEPTH, NULL, 2, 0);	//xTaskCreatePinnedToCore
	}
	else{
		ESP_LOGE(TAG, "SHT31 Sensor Init Failed" );
	}
}
