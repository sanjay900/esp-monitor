
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

void init_coms(void);
void init_adaptor(esp_netif_t *eth_netif);
bool isConnected;
int s_retry_num;
void got_ip_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);