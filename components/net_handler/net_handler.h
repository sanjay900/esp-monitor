
#include "tcpip_adapter.h"
#include "esp_eth.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

void init_coms(void);
void init_adaptor(void);
static bool isConnected;
static int s_retry_num;