#include "mqtt.h"
#include "mqtt_client.h"
#include "esp_system.h"
#include "config.h"
#include "esp_log.h"
static const char *TAG = "Environment Sensor - MQTT Handler";

static esp_mqtt_client_handle_t mqttClient;

static bool mqttStatus = 0;
static char mqttTopic[30];
static char humTopic[30];
static char tempTopic[30];
/** Event handler for MQTT events */
static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event) {
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
    return ESP_OK;
}
void publish(float temperature, float humidity) {
	char temperatureChar[64], humidityChar[64];
	int msg_id;
    //  TODO: we need a structure for a sensor here, and a function per sensor that gets messages to publish
    sprintf(temperatureChar, "%f", temperature);
    sprintf(humidityChar, "%f", humidity);
    msg_id = esp_mqtt_client_publish(mqttClient, tempTopic, temperatureChar, 0,
                                     1, 0);
    ESP_LOGD(TAG, "sent publish successful, msg_id=%d", msg_id);
    msg_id =
        esp_mqtt_client_publish(mqttClient, humTopic, humidityChar, 0, 1, 0);
    ESP_LOGD(TAG, "sent publish successful, msg_id=%d", msg_id);
}
void mqtt_send(void *pvParameters) {
	char temperatureChar[64], humidityChar[64];
	int msg_id;
	TickType_t last_wakeup = xTaskGetTickCount();

	while (1) {
		if(mqttStatus){
            //TODO: we need a structure for a sensor here, and a function per sensor that gets messages to publish
			// sprintf(temperatureChar, "%f", temperature);
			// sprintf(humidityChar, "%f", humidity);
			// msg_id = esp_mqtt_client_publish(mqttClient, tempTopic, temperatureChar, 0, 1, 0);
			// ESP_LOGD(TAG, "sent publish successful, msg_id=%d", msg_id);
			// msg_id = esp_mqtt_client_publish(mqttClient, humTopic, humidityChar, 0, 1, 0);
			// ESP_LOGD(TAG, "sent publish successful, msg_id=%d", msg_id);
		}
		else{
			//reconnect
			// while(esp_mqtt_client_reconnect(mqttClient) != ESP_OK);
		}
		
		// Wait until 2 seconds (cycle time) are over.
		vTaskDelayUntil(&last_wakeup, config_data.refresh_rate / portTICK_PERIOD_MS);
	}

}

/** INIT MQTT **/
static void mqtt_app_start(void)
{
    ESP_LOGI(TAG, "%s", config_data.mqtt_ip);
    esp_mqtt_client_config_t mqtt_cfg = {
        .uri = config_data.mqtt_ip,
        .event_handle = mqtt_event_handler
    };

    mqttClient = esp_mqtt_client_init(&mqtt_cfg);
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
void mqtt_init() {
	mqtt_app_start();
	xTaskCreatePinnedToCore(mqtt_send, "mqtt_send", TASK_STACK_DEPTH, NULL, 1, NULL, 1);
}