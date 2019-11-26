#include "net_handler.h"
static const char *TAG = "Environment Sensor";

static bool isConnected = 0;
static int s_retry_num = 0;
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

	if (event_id == IP_EVENT_STA_GOT_IP) { // WIFI
		s_retry_num = 0;	
	}
	isConnected = 1;
}

/** INIT Communication **/
void init_coms(void){
	// GENERAL
	tcpip_adapter_init();
}