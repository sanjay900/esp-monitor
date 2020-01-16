#include "display.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "ssd1306.h"
#include "ssd1306_default_if.h"
#include "ssd1306_draw.h"
#include "ssd1306_font.h"
#include "config.h"
#include "esp_netif.h"
#include "lwip/err.h"
#include "lwip/sys.h"

static const char *TAG = "Environment Sensor";
#define USE_I2C_DISPLAY
static const int I2CDisplayAddress = 0x3C;
static const int I2CDisplayWidth = 128;
static const int I2CDisplayHeight = 64;
static const int I2CResetPin = 16;
struct SSD1306_Device I2CDisplay;
/** Display TASK */
void display_task(void *pvParameters) {
	TickType_t last_wakeup = xTaskGetTickCount();
	while (1) {
		SSD1306_Clear(&I2CDisplay, SSD_COLOR_BLACK);
		// char sensorData[36], ipAdd[18];
		// TODO: IP Stuff has changed, also, do we actually plan on using screens with the wesp32?
// 		tcpip_adapter_ip_info_t ipInfo;
// #ifdef ACTIVE_SHT31
// 		sprintf(sensorData, "T:%3.2f%cC  H:%3.2f%%", temperature,(char)176,humidity);
// #endif // ACTIVE_SHT31
// #ifdef ACTIVE_WIFI
// 		tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_STA, &ipInfo);
// #endif // ACTIVE_WIFI
// #ifdef ACTIVE_ETHERNET
// 		tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_ETH, &ipInfo);
// #endif // ACTIVE_ETHERNET
		// SSD1306_FontDrawString(&I2CDisplay, 0, 0, sensorData, SSD_COLOR_WHITE);
		//SSD1306_FontDrawString(&I2CDisplay, 0, I2CDisplayHeight/4, mqtt_status_str,SSD_COLOR_WHITE);
		//SSD1306_FontDrawString(&I2CDisplay, 0, 2*I2CDisplayHeight/4, config_data.location, SSD_COLOR_WHITE);
		// sprintf(ipAdd,IPSTR, IP2STR(&ipInfo.ip));
		// SSD1306_FontDrawString(&I2CDisplay, 0, 3*I2CDisplayHeight/4, ipAdd, SSD_COLOR_WHITE);
		SSD1306_Update(&I2CDisplay);
		// Wait until 2 seconds (cycle time) are over.
		vTaskDelayUntil(&last_wakeup, config_data.refresh_rate / portTICK_PERIOD_MS);
	}
}

void display_init(void) {
    // Init Screen
	ESP_LOGI(TAG, "Screen Initialize" );
	if( SSD1306_I2CMasterInitDefault( ) == true ){
		if( SSD1306_I2CMasterAttachDisplayDefault( &I2CDisplay, I2CDisplayWidth, I2CDisplayHeight, I2CDisplayAddress, I2CResetPin ) == true ){
			SSD1306_Clear(&I2CDisplay, SSD_COLOR_BLACK);
			SSD1306_SetFont(&I2CDisplay, &Font_droid_sans_fallback_11x13);
			ESP_LOGI(TAG, "Screen Init OK" );
			SSD1306_FontDrawAnchoredString( &I2CDisplay, TextAnchor_NorthWest, "Waiting for connection...", SSD_COLOR_WHITE );
			SSD1306_Update( &I2CDisplay );
		}
		else{
			ESP_LOGE(TAG, "Screen attach failed" );
		}
	}
	else{
		ESP_LOGE(TAG, "Screen I2C init failed" );
	}
	// Create a user task that uses the display, low priority.
	xTaskCreate(display_task, "display_task", TASK_STACK_DEPTH, NULL, 1, 0);
}