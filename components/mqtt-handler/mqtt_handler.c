
#include "mqtt_handler.h"
#include "ssd1306.h"
#include "esp_event.h"
bool subscribed = false;
esp_mqtt_client_handle_t client;
mqtt_event_callback_t event_handle;
static const char *TAG = "Environment Sensor";

void init_mqtt(mqtt_event_callback_t event_handler, const char *uri) {
  event_handle = event_handler;
  const esp_mqtt_client_config_t mqtt_cfg = {
      .uri = uri, .event_handle = event_handler};
  client = esp_mqtt_client_init(&mqtt_cfg);
  esp_mqtt_client_start(client);
}

void publish(const char *data) {
 int msg_id =
      esp_mqtt_client_publish(client, "/topic/qos0", data, sizeof(data), 0, 0);
  ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
}