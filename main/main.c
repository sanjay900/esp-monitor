
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "cgi.h"
#include "config.h"
#include "d7s.h"
#include "driver/i2c.h"
#include "esp_eth.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "espfs.h"
#include "espfs_image.h"
#include "libesphttpd/auth.h"
#include "libesphttpd/captdns.h"
#include "libesphttpd/cgiflash.h"
#include "libesphttpd/cgiwebsocket.h"
#include "libesphttpd/cgiwifi.h"
#include "libesphttpd/httpd-espfs.h"
#include "libesphttpd/httpd-freertos.h"
#include "libesphttpd/httpd.h"
#include "libesphttpd/route.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "mqtt.h"
#include "net_handler.h"
#include "sensor.h"
#include "sht3x.h"
#include <libesphttpd/esp.h>


#define I2CSensor_BUS 0
#define I2CSensor_SCL_PIN 5
#define I2CSensor_SDA_PIN 33
#define I2CSensor_FREQ I2C_FREQ_400K
#define LISTEN_PORT 80u
#define MAX_CONNECTIONS 32u
#define OTA_FLASH_SIZE_K 1024
#define OTA_TAGNAME "generic"

CgiUploadFlashDef uploadParams = {
    .type = CGIFLASH_TYPE_FW,
    .fw1Pos = 0x1000,
    .fw2Pos = ((OTA_FLASH_SIZE_K * 1024) / 2) + 0x1000,
    .fwSize = ((OTA_FLASH_SIZE_K * 1024) / 2) - 0x1000,
    .tagName = OTA_TAGNAME};
static void websocketRecv(Websock *ws, char *data, int len, int flags) {
  // int i;
  // char buff[128];
  // sprintf(buff, "You sent: ");
  // for (i=0; i<len; i++) buff[i+10]=data[i];
  // buff[i+10]=0;
  // cgiWebsocketSend(&httpdFreertosInstance.httpdInstance,
  //                  ws, buff, strlen(buff), WEBSOCK_FLAG_NONE);
}
static void websocketConnect(Websock *ws) { ws->recvCb = websocketRecv; }
static char connectionMemory[sizeof(RtosConnType) * MAX_CONNECTIONS];
static HttpdFreertosInstance httpdFreertosInstance;
HttpdBuiltInUrl builtInUrls[] = {
    ROUTE_REDIRECT("/", "/index.tpl"),
    ROUTE_CGI("/config.cgi", cgiConfig),
    ROUTE_REDIRECT("/flash", "/flash/index.tpl"),
    ROUTE_REDIRECT("/flash/", "/flash/index.tpl"),
    ROUTE_TPL("/flash/index.tpl", tplCurrentConfig),
    ROUTE_CGI("/flash/flashinfo.json", cgiGetFlashInfo),
    ROUTE_CGI("/flash/setboot", cgiSetBoot),
    ROUTE_CGI_ARG("/flash/upload", cgiUploadFirmware, &uploadParams),
    ROUTE_CGI_ARG("/flash/erase", cgiEraseFlash, &uploadParams),
    ROUTE_CGI("/flash/reboot", cgiRebootFirmware),
    ROUTE_REDIRECT("/wifi", "/wifi/wifi.tpl"),
    ROUTE_REDIRECT("/wifi/", "/wifi/wifi.tpl"),
    ROUTE_CGI("/wifi/wifiscan.cgi", cgiWiFiScan),
    ROUTE_TPL("/wifi/wifi.tpl", tplWlan),
    ROUTE_CGI("/wifi/connect.cgi", cgiWiFiConnect),
    ROUTE_CGI("/wifi/connstatus.cgi", cgiWiFiConnStatus),
    ROUTE_CGI("/wifi/setmode.cgi", cgiWiFiSetMode),
    ROUTE_CGI("/wifi/startwps.cgi", cgiWiFiStartWps),
    ROUTE_CGI("/wifi/ap", cgiWiFiAPSettings),
    ROUTE_TPL("/index.tpl", tplCurrentConfig),
    ROUTE_TPL("/log.tpl", tplCurrentConfig),
    ROUTE_TPL("/sensor_data.tpl", tplCurrentConfig),
    ROUTE_WS("/websocket/log.cgi", websocketConnect),
    ROUTE_WS("/websocket/sensors.cgi", websocketConnect),
    ROUTE_FILESYSTEM(),
    ROUTE_END()};
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
static char log_print_buffer[512];
static vprintf_like_t orig_esp_log = NULL;
static int vprintf_into_spiffs(const char *szFormat, va_list args) {
  orig_esp_log(szFormat, args);
  // write evaluated format string into buffer
  int ret =
      vsnprintf(log_print_buffer, sizeof(log_print_buffer), szFormat, args);
  cgiWebsockBroadcast(&httpdFreertosInstance.httpdInstance,
                      "/websocket/log.cgi", log_print_buffer,
                      strlen(log_print_buffer), WEBSOCK_FLAG_NONE);
  return ret;
}
static char sensor_print_buffer[512];
void send_sensor(message_t *msg) {
  sprintf(sensor_print_buffer, "%s: %s", msg->topic, msg->message);
  cgiWebsockBroadcast(&httpdFreertosInstance.httpdInstance,
                      "/websocket/sensors.cgi", sensor_print_buffer,
                      strlen(sensor_print_buffer), WEBSOCK_FLAG_NONE);
}

void app_main(void) {

  orig_esp_log = esp_log_set_vprintf(vprintf_into_spiffs);
  ESP_LOGI(TAG, "Initializing processes");
  ESP_LOGI(TAG, "IDF-Version");
  ESP_LOGI(TAG, "%s", IDF_VER);
  esp_log_level_set("*", ESP_LOG_INFO);
  esp_log_level_set("MQTT_CLIENT", ESP_LOG_VERBOSE);
  esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
  esp_log_level_set("TRANSPORT_TCP", ESP_LOG_VERBOSE);
  esp_log_level_set("TRANSPORT_SSL", ESP_LOG_VERBOSE);
  esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
  esp_log_level_set("OUTBOX", ESP_LOG_VERBOSE);
  esp_log_level_set("wifi", ESP_LOG_VERBOSE);
  init_config();
  // Coms
  init_coms();
  // Wait for connection
  ESP_LOGI(TAG, "Waiting for connection before continuing to init");
  while (isConnected == 0) {
    // ESP_LOGI(TAG, "." );
    vTaskDelay(10);
  };

  
  EspFsConfig espfs_conf = {
      .memAddr = espfs_image_bin,
  };
  EspFs *fs = espFsInit(&espfs_conf);
  httpdRegisterEspfs(fs);

  httpdFreertosInit(&httpdFreertosInstance, builtInUrls, LISTEN_PORT,
                    connectionMemory, MAX_CONNECTIONS, HTTPD_FLAG_NONE);
  httpdFreertosStart(&httpdFreertosInstance);
  
  initialise_sensors();

  ESP_LOGI(TAG, "MQTT Initialize");
  mqtt_init();
  on_message = send_sensor;

  ESP_LOGI(TAG, "Initializing done");
}
