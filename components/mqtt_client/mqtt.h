#ifdef ESP_PLATFORM // ESP32 (ESP-IDF)
	#define TASK_STACK_DEPTH 2048	// user task stack depth for ESP32
#else // ESP8266 (esp-open-rtos)
	#define TASK_STACK_DEPTH 256	// user task stack depth for ESP8266
#endif // ESP_PLATFORM
void mqtt_init(void);
void publish(float temperature, float humidity);