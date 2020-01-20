#include "mqtt.h"
#include "mqtt_client.h"
#include "esp_system.h"
#include "config.h"
#include "esp_log.h"
#include "sensor.h"
static const char *TAG = "Environment Sensor - MQTT Handler";

static esp_mqtt_client_handle_t mqttClient;

static bool mqttStatus = 0;
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
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA");
            ESP_LOGI(TAG, "TOPIC=%.*s", event->topic_len, event->topic);
            ESP_LOGI(TAG, "DATA=%.*s", event->data_len, event->data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
            break;
        default:
            break;
    }
    return ESP_OK;
}
void mqtt_send(void *pvParameters) {
	TickType_t last_wakeup = xTaskGetTickCount();
    message_t msg;
	while (1) {
        for (uint8_t i =0; i < current_sensor; i++) {
            while (xQueueReceive(sensors[i]->messages, &msg, (TickType_t)0)) {
                esp_mqtt_client_publish(mqttClient, msg.topic, msg.message, 0, 1, 0);
            }
        }		
		// Wait until 2 seconds (cycle time) are over.
		vTaskDelayUntil(&last_wakeup, 500 / portTICK_PERIOD_MS);
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
}
void mqtt_init() {
	mqtt_app_start();
    xTaskCreatePinnedToCore(mqtt_send, "mqtt_send", TASK_STACK_DEPTH, NULL, 1, NULL, 1);
}