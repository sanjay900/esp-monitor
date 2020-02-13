#include "net_handler.h"
static const char *TAG = "net_handler";

bool isConnected = 0;
/** Event handler for Got IP events */
void got_ip_event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data) {
  if (event_id == IP_EVENT_ETH_GOT_IP) {
    isConnected = 1;
  }
}

/** INIT Communication **/
void init_coms(void) {
  // GENERAL
  // Initialize TCP/IP network interface (should be called only once in
  // application)
  ESP_ERROR_CHECK(esp_netif_init());
  // Create default event loop that running in background
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP,
                                             &got_ip_event_handler, NULL));
  ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                             &got_ip_event_handler, NULL));
  init_eth_adaptor();
  init_wifi_adaptor();
}