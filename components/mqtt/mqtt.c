
#include "mqtt.h";
#include "mqtt_client.h";
bool subscribed = false;
esp_mqtt_client_handle_t client;
mqtt_event_callback_t event_handle;
static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event) {
  event_handle(event);
  esp_mqtt_client_handle_t client = event->client;
  int msg_id;
  // your_context_t *context = event->context;
  switch (event->event_id) {
  case MQTT_EVENT_CONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
    msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
    ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
    SSD1306_FontDrawAnchoredString(&I2CDisplay, TextAnchor_North,
                                   "MQTT Connected", SSD_COLOR_WHITE);
    SSD1306_Update(&I2CDisplay);
    break;
  case MQTT_EVENT_DISCONNECTED:
    ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
    break;

  case MQTT_EVENT_SUBSCRIBED:
    ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
    subscribed = true;
    break;
  case MQTT_EVENT_UNSUBSCRIBED:
    ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_BEFORE_CONNECT:
    ESP_LOGI(TAG, "MQTT_EVENT_BEFORE_CONNECT, msg_id=%d", event->msg_id);
  case MQTT_EVENT_PUBLISHED:
    ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
    break;
  case MQTT_EVENT_DATA:
    ESP_LOGI(TAG, "MQTT_EVENT_DATA");
    printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
    printf("DATA=%.*s\r\n", event->data_len, event->data);
    break;
  case MQTT_EVENT_ERROR:
    ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
    break;
  }
  return ESP_OK;
}

void init_mqtt(mqtt_event_callback_t event_handler, const char *uri) {
  event_handle = event_handler;
  const esp_mqtt_client_config_t mqtt_cfg = {
      .uri = uri, .event_handle = mqtt_event_handler};
  client = esp_mqtt_client_init(mqtt_cfg);
  esp_mqtt_client_start(client);
}

void publish(const *data) {
  msg_id =
      esp_mqtt_client_publish(client, "/topic/qos0", data, sizeof(data), 0, 0);
  ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
}