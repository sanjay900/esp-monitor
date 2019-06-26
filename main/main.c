
#include "esp_event.h"
#include "mqtt.h"
#include "sht3x.h"
#include "ssd1306.h"
#include "ssd1306_default_if.h"
#include "ssd1306_draw.h"
#include "ssd1306_font.h"
#include "tcpip_adapter.h"
#include "wifi_manager.h"
#include <stdio.h>
#include <string.h>
static const char *TAG = "Environment Sensor";
#ifdef ESP_PLATFORM // ESP32 (ESP-IDF)

// user task stack depth for ESP32
#define TASK_STACK_DEPTH 2048

#else // ESP8266 (esp-open-rtos)

// user task stack depth for ESP8266
#define TASK_STACK_DEPTH 256

#endif // ESP_PLATFORM
#define I2C_BUS 1
#define I2C_SCL_PIN 22
#define I2C_SDA_PIN 21
#define I2C_FREQ I2C_FREQ_100K

/* -- user tasks --------------------------------------------------- */

static sht3x_sensor_t *sensor; // sensor device data structure
struct SSD1306_Device I2CDisplay;
static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event) {
  esp_mqtt_client_handle_t client = event->client;
  int msg_id;
  char msg = "";
  // your_context_t *context = event->context;
  switch (event->event_id) {
  case MQTT_EVENT_CONNECTED:
  msg = "MQTT Connected";
    break;
  case MQTT_EVENT_DISCONNECTED:
  msg = "MQTT Disconnected";
    break;
  }
  SSD1306_FontDrawAnchoredString(&I2CDisplay, TextAnchor_North, msg,
                                 SSD_COLOR_WHITE);
  SSD1306_Update(&I2CDisplay);
  return ESP_OK;
}

void init_screen(void) {
  SSD1306_I2CMasterInitDefault();
  SSD1306_I2CMasterAttachDisplayDefault(&I2CDisplay, 128, 64, 0x3C, 16);
  SSD1306_Clear(&I2CDisplay, SSD_COLOR_BLACK);
  SSD1306_SetFont(&I2CDisplay, &Font_droid_sans_fallback_11x13);
}

void addr_event_handler(void *arg, esp_event_base_t base, int32_t event_id,
                        void *data) {
  const ip_event_got_ip_t *event = (const ip_event_got_ip_t *)data;
  char ipData[255];
  sprintf(ipData, IPSTR "\n" IPSTR "\n" IPSTR, IP2STR(&event->ip_info.ip),
          IP2STR(&event->ip_info.netmask), IP2STR(&event->ip_info.gw));
  SSD1306_FontDrawAnchoredString(&I2CDisplay, TextAnchor_Center, ipData,
                                 SSD_COLOR_WHITE);
  SSD1306_Update(&I2CDisplay);
}
void user_task(void *pvParameters) {
  float temperature;
  float humidity;

  // Start periodic measurements with 1 measurement per second.
  sht3x_start_measurement(sensor, sht3x_periodic_1mps, sht3x_high);

  // Wait until first measurement is ready (constant time of at least 30 ms
  // or the duration returned from *sht3x_get_measurement_duration*).
  vTaskDelay(sht3x_get_measurement_duration(sht3x_high));

  TickType_t last_wakeup = xTaskGetTickCount();

  while (1) {
    // Get the values and do something with them.
    if (sht3x_get_results(sensor, &temperature, &humidity))
      ESP_LOGI(TAG, "%.3f SHT3x Sensor: %.2f °C, %.2f %%\n",
               (double)sdk_system_get_time() * 1e-3, temperature, humidity);

    // Wait until 2 seconds (cycle time) are over.
    vTaskDelayUntil(&last_wakeup, 2000 / portTICK_PERIOD_MS);
  }
}
void app_main(void) {
  manager_main(addr_event_handler);
  init_screen();
  SSD1306_FontDrawAnchoredString(&I2CDisplay, TextAnchor_North,
                                 "MQTT Disconnected", SSD_COLOR_WHITE);
  SSD1306_Update(&I2CDisplay);
  const esp_mqtt_client_config_t mqtt_cfg = {
      .uri = "mqtt://172.22.2.163:1883", .event_handle = mqtt_event_handler};
      
  
  i2c_init(I2C_BUS, I2C_SCL_PIN, I2C_SDA_PIN, I2C_FREQ);

  // Create the sensors, multiple sensors are possible.
  if ((sensor = sht3x_init_sensor(I2C_BUS, SHT3x_ADDR_1))) {
    // Create a user task that uses the sensors.
    xTaskCreate(user_task, "user_task", TASK_STACK_DEPTH, NULL, 2, 0);
  }
}
