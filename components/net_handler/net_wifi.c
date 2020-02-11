#include "libesphttpd/cgiwifi.h"
#include "net_handler.h"
#include "nvs_flash.h"
#include <string.h>
#define NVS_NAMESPACE "nvs"
nvs_handle my_nvs_handle;
#define NET_CONF_KEY "netconf"
#define DEFAULT_WIFI_MODE WIFI_MODE_AP
static const char *TAG = "net_wifi.c";
bool started = false;

static system_event_id_t esp_event_legacy_wifi_event_id(int32_t event_id) {
  switch (event_id) {
  case WIFI_EVENT_SCAN_DONE:
    return SYSTEM_EVENT_SCAN_DONE;

  case WIFI_EVENT_STA_DISCONNECTED:
    return SYSTEM_EVENT_STA_DISCONNECTED;

  case WIFI_EVENT_STA_WPS_ER_FAILED:
    return SYSTEM_EVENT_STA_WPS_ER_FAILED;

  case WIFI_EVENT_STA_WPS_ER_TIMEOUT:
    return SYSTEM_EVENT_STA_WPS_ER_TIMEOUT;

  case WIFI_EVENT_STA_WPS_ER_PIN:
    return SYSTEM_EVENT_STA_WPS_ER_PIN;

  default:
    return SYSTEM_EVENT_MAX;
  }
}

static system_event_id_t esp_event_legacy_ip_event_id(int32_t event_id) {
  switch (event_id) {
  case IP_EVENT_STA_GOT_IP:
    return SYSTEM_EVENT_STA_GOT_IP;

  case IP_EVENT_GOT_IP6:
    return SYSTEM_EVENT_GOT_IP6;

  default:
    return SYSTEM_EVENT_MAX;
  }
}

static system_event_id_t esp_event_legacy_event_id(esp_event_base_t event_base,
                                                   int32_t event_id) {
  if (event_base == WIFI_EVENT) {
    return esp_event_legacy_wifi_event_id(event_id);
  } else if (event_base == IP_EVENT) {
    return esp_event_legacy_ip_event_id(event_id);
  } else {
    ESP_LOGE(TAG, "invalid event base %s", event_base);
    return SYSTEM_EVENT_MAX;
  }
}
static void app_event_handler(void *arg, esp_event_base_t event_base,
                              int32_t event_id, void *event_data) {
  switch (event_id) {
  case WIFI_EVENT_STA_START:
    if (!started) {
      started = true;
      esp_wifi_connect();
    }
    // esp_wifi_connect(); Calling this unconditionally would interfere with
    // the WiFi CGI.
    break;
  case IP_EVENT_STA_GOT_IP: {
    isConnected = 1;
    tcpip_adapter_ip_info_t sta_ip_info;
    wifi_config_t sta_conf;
    printf("~~~~~STA~~~~~"
           "\n");
    if (esp_wifi_get_config(TCPIP_ADAPTER_IF_STA, &sta_conf) == ESP_OK) {
      printf("ssid: %s"
             "\n",
             sta_conf.sta.ssid);
    }
    printf("~~~~~~~~~~~~~"
           "\n");
    break;
  }
  case WIFI_EVENT_STA_CONNECTED:
    break;
  case WIFI_EVENT_STA_DISCONNECTED: {
    wifi_event_sta_disconnected_t *disconnected =
        (wifi_event_sta_disconnected_t *)event_data;
    // This is a workaround as ESP32 WiFi libs don't currently
    // auto-reassociate.
    //  Skip reconnect if disconnect was deliberate or authentication
    //  failed.
    switch (disconnected->reason) {
    case WIFI_REASON_ASSOC_LEAVE:
    case WIFI_REASON_AUTH_FAIL:
      break;
    default:
      esp_wifi_connect();
      break;
    }
    break;
  }
  case WIFI_EVENT_AP_START: {
    tcpip_adapter_ip_info_t ap_ip_info;
    wifi_config_t ap_conf;
    printf("~~~~~AP~~~~~"
           "\n");
    if (esp_wifi_get_config(TCPIP_ADAPTER_IF_AP, &ap_conf) == ESP_OK) {
      printf("ssid: %s"
             "\n",
             ap_conf.ap.ssid);
      if (ap_conf.ap.authmode != WIFI_AUTH_OPEN)
        printf("pass: %s"
               "\n",
               ap_conf.ap.password);
    }

    if (tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_AP, &ap_ip_info) == ESP_OK) {
      printf("IP:" IPSTR "\n", IP2STR(&ap_ip_info.ip));
      printf("MASK:" IPSTR "\n", IP2STR(&ap_ip_info.netmask));
      printf("GW:" IPSTR "\n", IP2STR(&ap_ip_info.gw));
    }
    printf("~~~~~~~~~~~~"
           "\n");
  } break;
  case WIFI_EVENT_AP_STACONNECTED: {
    wifi_event_ap_staconnected_t *sta_connected =
        (wifi_event_ap_staconnected_t *)event_data;
    ESP_LOGI(TAG, "station:" MACSTR " join,AID=%d", MAC2STR(sta_connected->mac),
             sta_connected->aid);

    break;
  }
  case WIFI_EVENT_AP_STADISCONNECTED: {
    wifi_event_ap_stadisconnected_t *sta_disconnected =
        (wifi_event_ap_stadisconnected_t *)event_data;
    ESP_LOGI(TAG, "station:" MACSTR "leave,AID=%d",
             MAC2STR(sta_disconnected->mac), sta_disconnected->aid);

    break;
  }
  case WIFI_EVENT_SCAN_DONE:

    break;
  default:
    break;
  }
  system_event_t event;
  event.event_id = esp_event_legacy_event_id(event_base, event_id);

  if (event_id == WIFI_EVENT_SCAN_DONE) {
    memcpy(&event.event_info, event_data, sizeof(system_event_sta_scan_done_t));
  }
  cgiWifiEventCb(&event);
}

void init_wifi(bool factory_defaults) {

  ESP_ERROR_CHECK(initCgiWifi()); // Initialise wifi configuration CGI
  wifi_mode_t old_mode;
  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_wifi_init(&cfg));

  ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_FLASH));
  ESP_ERROR_CHECK(esp_wifi_get_mode(&old_mode));

  if (factory_defaults) {
    old_mode = DEFAULT_WIFI_MODE;
  }

  if (old_mode == WIFI_MODE_APSTA || old_mode == WIFI_MODE_STA) {
    //// STA settings
    wifi_config_t factory_sta_config = {
        .sta = {
            .ssid = CONFIG_ESP_WIFI_SSID,
            .password = CONFIG_ESP_WIFI_PASSWORD,
        }};
    wifi_config_t sta_stored_config;
    // esp_wifi_set_mode(WIFI_MODE_APSTA); // must enable modes before trying
    // esp_wifi_get_config()
    ESP_ERROR_CHECK(esp_wifi_get_config(ESP_IF_WIFI_STA, &sta_stored_config));

    if (factory_defaults && strlen((char *)factory_sta_config.sta.ssid) != 0) {
      // load factory default STA config
      ESP_LOGI(TAG, "Using factory-default WiFi STA configuration, ssid: %s",
               factory_sta_config.sta.ssid);
      ESP_ERROR_CHECK(
          esp_wifi_set_config(ESP_IF_WIFI_STA, &factory_sta_config));
    } else if (strlen((char *)sta_stored_config.sta.ssid) != 0) {
      ESP_LOGI(TAG, "Using WiFi STA configuration from NVS, ssid: %s",
               sta_stored_config.sta.ssid);
      ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &sta_stored_config));
    } else {
      ESP_LOGW(TAG, "No WiFi STA configuration available");
      if (old_mode == WIFI_MODE_APSTA)
        esp_wifi_set_mode(WIFI_MODE_AP); // remove STA mode
      if (old_mode == WIFI_MODE_STA)
        esp_wifi_set_mode(WIFI_MODE_NULL); // remove STA mode
    }
  }

  if (old_mode == WIFI_MODE_APSTA || old_mode == WIFI_MODE_AP) {
    //// AP settings
    wifi_config_t factory_ap_config;
    {
      strncpy((char *)(&factory_ap_config.ap.ssid), "ESP",
              (sizeof((wifi_ap_config_t *)0)->ssid));
      factory_ap_config.ap.ssid_len =
          0; // 0: use null termination to determine size
      factory_ap_config.ap.channel = 6;
      factory_ap_config.ap.authmode =
          WIFI_AUTH_OPEN; // WIFI_AUTH_WPA_WPA2_PSK; //WIFI_AUTH_OPEN;
      // strncpy((char *)(&factory_ap_config.ap.password), DEFAULT_WIFI_AP_PASS,
      // (sizeof((wifi_ap_config_t *)0)->password));
      factory_ap_config.ap.ssid_hidden = 0;
      factory_ap_config.ap.max_connection = 4;
      factory_ap_config.ap.beacon_interval = 100;
    }
    wifi_config_t ap_stored_config;
    ESP_ERROR_CHECK(esp_wifi_get_config(ESP_IF_WIFI_AP, &ap_stored_config));

    if (factory_defaults && strlen((char *)factory_ap_config.ap.ssid) != 0) {

      // load factory default STA config
      ESP_LOGI(TAG, "Using factory-default WiFi AP configuration, ssid: %s",
               factory_ap_config.ap.ssid);
      ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &factory_ap_config));
    } else if (strlen((char *)ap_stored_config.ap.ssid) != 0) {
      ESP_LOGI(TAG, "Using WiFi AP configuration from NVS, ssid: %s",
               ap_stored_config.ap.ssid);
      ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &ap_stored_config));
    } else {
      ESP_LOGW(TAG, "No WiFi AP configuration available");
      if (old_mode == WIFI_MODE_APSTA)
        esp_wifi_set_mode(WIFI_MODE_STA); // remove AP mode
      if (old_mode == WIFI_MODE_AP)
        esp_wifi_set_mode(WIFI_MODE_NULL); // remove AP mode
    }
  }

  ESP_ERROR_CHECK(esp_wifi_start());
}
void init_wifi_adaptor() {
  // Init WIFI
  ESP_LOGD(TAG, "Initializing WIFI...");
  esp_netif_create_default_wifi_sta();
  esp_err_t err;
  // Init NVS
  err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    // NVS partition was truncated and needs to be erased
    // Retry nvs_flash_init
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);
  uint32_t net_configured = 0; // value will default to 0, if not set yet in NVS
  ESP_LOGI(TAG, "Opening NVS handle ");
  err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &my_nvs_handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
  } else {

    // Read NVS
    ESP_LOGI(TAG, "Reading network initialization from NVS");
    err = nvs_get_u32(my_nvs_handle, NET_CONF_KEY, &net_configured);
    switch (err) {
    case ESP_OK:
      ESP_LOGI(TAG, "nvs init = %d", net_configured);
      break;
    case ESP_ERR_NVS_NOT_FOUND:
      ESP_LOGI(TAG, "nvs init not found, initializing now.");
      break;
    default:
      ESP_LOGE(TAG, "Error (%s) reading!\n", esp_err_to_name(err));
    }
  }

  ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                             &app_event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID,
                                             &app_event_handler, NULL));

  init_wifi(!net_configured); // Start Wifi, restore factory wifi settings if
                              // not initialized

  if (!net_configured) { // If wasn't initialized, now we are initialized. Write
                         // it to NVS.
    net_configured = 1;
    ESP_LOGI(TAG, "Writing init to NVS");
    ESP_ERROR_CHECK(nvs_set_u32(my_nvs_handle, NET_CONF_KEY, net_configured));
    // After setting any values, nvs_commit() must be called to ensure changes
    // are written to flash storage.
    ESP_LOGI(TAG, "Committing updates in NVS");
    ESP_ERROR_CHECK(nvs_commit(my_nvs_handle));
    // Close NVS
    // nvs_close(my_nvs_handle); - don't close if handle is shared by cgi_NVS
  }
}