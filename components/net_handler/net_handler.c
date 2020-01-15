#include "net_handler.h"
static const char *TAG = "Environment Sensor - Net Handler";

bool isConnected = 0;
int s_retry_num = 0;
/** Event handler for Got IP events */
void got_ip_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data){

    ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
    const esp_netif_ip_info_t *ip_info = &event->ip_info;

	ESP_LOGI(TAG, "Got IP Address");
    ESP_LOGI(TAG, "~~~~~~~~~~~");
    ESP_LOGI(TAG, "IP:" IPSTR, IP2STR(&ip_info->ip));
    ESP_LOGI(TAG, "MASK:" IPSTR, IP2STR(&ip_info->netmask));
    ESP_LOGI(TAG, "GW:" IPSTR, IP2STR(&ip_info->gw));
    ESP_LOGI(TAG, "~~~~~~~~~~~");

	if (event_id == IP_EVENT_STA_GOT_IP) {
		s_retry_num = 0;	
	}
	isConnected = 1;
}

/** INIT Communication **/
void init_coms(void){
	// GENERAL
	// Initialize TCP/IP network interface (should be called only once in application)
    ESP_ERROR_CHECK(esp_netif_init());
    // Create default event loop that running in background
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
    esp_netif_t *eth_netif = esp_netif_new(&cfg);
    // Set default handlers to process TCP/IP stuffs
    ESP_ERROR_CHECK(esp_eth_set_default_handlers(eth_netif));
    init_adaptor(eth_netif);
}