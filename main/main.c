
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

#ifdef ACTIVE_WIFI
	#define ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
	#define ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
	#define WIFI_MAXIMUM_RETRY 5
	/* FreeRTOS event group to signal when we are connected*/
	static EventGroupHandle_t s_wifi_event_group;
	/* The event group allows multiple bits for each event, but we only care about one event 
	 * - are we connected to the AP with an IP? */
	const int WIFI_CONNECTED_BIT = BIT0;
	static int s_retry_num = 0;
#endif // ACTIVE_WIFI

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

typedef struct{
	char f_version[4];
	char h_version[4];
	int id;
	char name[20];
	char location[20];
	char sensor_type[4];
	int refresh_rate;
	int alarm_treshold_min;
	int alarm_treshold_max;
	char esp32_ip[15];
	char mqtt_ip[30];
	int log_activ;
	int log_days;
} config_struct;

static config_struct config_data;

/* -- Declaration --------------------------------------------------- */
esp_err_t start_server(void);
static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
static void eth_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void got_ip_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

/* -- INIT --------------------------------------------------- */

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

/** INIT Communication **/
void init_coms(void){
	// GENERAL
	tcpip_adapter_init();

#ifdef ACTIVE_WIFI
	//Init WIFI
	ESP_LOGD(TAG, "Initializing WIFI...");
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &got_ip_event_handler, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = ESP_WIFI_SSID,
            .password = ESP_WIFI_PASS
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGD(TAG, "wifi_init_sta finished.");
    ESP_LOGI(TAG, "connect to ap SSID:%s password:%s",
             ESP_WIFI_SSID, ESP_WIFI_PASS);
#endif // ACTIVE_WIFI

#ifdef ACTIVE_ETHERNET
    ESP_LOGD(TAG, "Initializing ETHERNET...");
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(tcpip_adapter_set_default_eth_handlers());
    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &got_ip_event_handler, NULL));

    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
	
	eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
	/* Set the PHY address*/
	phy_config.phy_addr = 0;
	phy_config.phy_power_enable(false);
	
    esp_eth_mac_t *mac = esp_eth_mac_new_esp32(&mac_config);
    esp_eth_phy_t *phy = esp_eth_phy_new_lan8720(&phy_config);

    esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy);
    esp_eth_handle_t eth_handle = NULL;
    ESP_ERROR_CHECK(esp_eth_driver_install(&config, &eth_handle));
	ESP_LOGD(TAG, "ETHERNET initialized");
#endif // ACTIVE_ETHERNET
}

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

#ifdef ACTIVE_WIFI
/** Event handler for Wifi events */
static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data){
    if (event_id == WIFI_EVENT_STA_START) {
		esp_wifi_connect();
    } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < WIFI_MAXIMUM_RETRY) {
			esp_wifi_connect();
            xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
            s_retry_num++;
			isConnected = 0;
            ESP_LOGD(TAG, "retry to connect to the AP");
        }
        ESP_LOGD(TAG,"connect to the AP fail");
    }
}
#endif // ACTIVE_WIFI

#ifdef ACTIVE_ETHERNET
/** Event handler for Ethernet events */
static void eth_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data){
    uint8_t mac_addr[6] = {0};
    /* we can get the ethernet driver handle from event data */
    esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;

    switch (event_id) {
    case ETHERNET_EVENT_CONNECTED:
        esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
        ESP_LOGD(TAG, "Ethernet Link Up");
        ESP_LOGD(TAG, "Ethernet HW Addr %02x:%02x:%02x:%02x:%02x:%02x",
                 mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
        break;
    case ETHERNET_EVENT_DISCONNECTED:
		isConnected = 0;
        ESP_LOGD(TAG, "Ethernet Link Down");
        break;
    case ETHERNET_EVENT_START:
        ESP_LOGD(TAG, "Ethernet Started");
        break;
    case ETHERNET_EVENT_STOP:
		isConnected = 0;
        ESP_LOGD(TAG, "Ethernet Stopped");
        break;
    default:
        break;
    }
}
#endif // ACTIVE_ETHERNET


/** Event handler for Got IP events */
static void got_ip_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data){

    ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
    tcpip_adapter_ip_info_t *ip_info = &event->ip_info;

	ESP_LOGI(TAG, "Got IP Address");
    ESP_LOGI(TAG, "~~~~~~~~~~~");
    ESP_LOGI(TAG, "IP:" IPSTR, IP2STR(&ip_info->ip));
    ESP_LOGI(TAG, "MASK:" IPSTR, IP2STR(&ip_info->netmask));
    ESP_LOGI(TAG, "GW:" IPSTR, IP2STR(&ip_info->gw));
    ESP_LOGI(TAG, "~~~~~~~~~~~");

#ifdef ACTIVE_WIFI	
	if (event_id == IP_EVENT_STA_GOT_IP) { // WIFI
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
		s_retry_num = 0;	
	}
#endif // ACTIVE_WIFI
#ifdef ACTIVE_ETHERNET
	if(event_id == IP_EVENT_ETH_GOT_IP) { // ETHERNET
		
	}
#endif // ACTIVE_ETHERNET
	isConnected = 1;
}

/** Event handler for MQTT events */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;    
	esp_mqtt_client_handle_t client = event->client;
    int msg_id;
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
	
	//Spiffs 
    ESP_ERROR_CHECK(init_spiffs());
	//Config
	load_config();
	//NVS
	ESP_ERROR_CHECK(init_NVS());
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
	sensor = sensor_sht31;
#endif // ACTIVE_SHT31	
	
	if (sensor) {
		ESP_LOGD(TAG, "Sensor Init OK" );
		// Create a user task that uses the sensors.
		xTaskCreate(sensor_read, "sensor_read", TASK_STACK_DEPTH, NULL, 2, 0);	//xTaskCreatePinnedToCore
	}
	else{
		ESP_LOGE(TAG, "Sensor Init Failed" );
	}
	
	
	ESP_LOGD(TAG, "MQTT Initialize");
	mqtt_app_start();
	xTaskCreate(mqtt_send, "mqtt_send", TASK_STACK_DEPTH, NULL, 1, 0);	//xTaskCreatePinnedToCore
	
	ESP_LOGD(TAG, "Initializing done" );
}
