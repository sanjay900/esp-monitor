#include "lwip/netif.h"
#include "net_handler.h"
#include "esp_netif_net_stack.h"
static const char *TAG = "Environment Sensor";

/** Event handler for Ethernet events */
static void eth_event_handler(void *arg, esp_event_base_t event_base,
                              int32_t event_id, void *event_data) {
  uint8_t mac_addr[6] = {0};
  /* we can get the ethernet driver handle from event data */
  esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;

  switch (event_id) {
  case ETHERNET_EVENT_CONNECTED:
    esp_eth_ioctl(eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr);
    ESP_LOGI(TAG, "Ethernet Link Up");
    ESP_LOGI(TAG, "Ethernet HW Addr %02x:%02x:%02x:%02x:%02x:%02x", mac_addr[0],
             mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
    break;
  case ETHERNET_EVENT_DISCONNECTED:
    isConnected = 0;
    ESP_LOGI(TAG, "Ethernet Link Down");
    break;
  case ETHERNET_EVENT_START:
    ESP_LOGI(TAG, "Ethernet Started");
    // netif_set_default((struct netif *)esp_netif_get_netif_impl(eth_netif));
    break;
  case ETHERNET_EVENT_STOP:
    isConnected = 0;
    ESP_LOGI(TAG, "Ethernet Stopped");
    break;
  default:
    break;
  }
}
void init_eth_adaptor() {
  ESP_LOGI(TAG, "Initializing ETHERNET...");

  esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
  eth_netif = esp_netif_new(&cfg);
  // Set default handlers to process TCP/IP stuffs
  ESP_ERROR_CHECK(esp_eth_set_default_handlers(eth_netif));
  ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ESP_EVENT_ANY_ID,
                                             &eth_event_handler, NULL));

  eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
  mac_config.smi_mdc_gpio_num = 16;
  mac_config.smi_mdio_gpio_num = 17;
  eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
  /* Set the PHY address*/
  phy_config.phy_addr = 0;
  phy_config.reset_gpio_num = 5;

  esp_eth_mac_t *mac = esp_eth_mac_new_esp32(&mac_config);
  esp_eth_phy_t *phy = esp_eth_phy_new_lan8720(&phy_config);

  esp_eth_config_t config = ETH_DEFAULT_CONFIG(mac, phy);
  esp_eth_handle_t eth_handle = NULL;
  ESP_ERROR_CHECK(esp_eth_driver_install(&config, &eth_handle));
  /* attach Ethernet driver to TCP/IP stack */
  ESP_ERROR_CHECK(
      esp_netif_attach(eth_netif, esp_eth_new_netif_glue(eth_handle)));

  /* start Ethernet driver state machine */
  ESP_ERROR_CHECK(esp_eth_start(eth_handle));
  ESP_LOGI(TAG, "ETHERNET initialized");
}