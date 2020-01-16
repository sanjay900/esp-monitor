#pragma once
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "esp8266_wrapper.h"
#include "config.h"
#define SENSOR_SHT3X 0
#define SENSOR_D7S 1 
#define MESSAGE_COUNT 10
typedef struct message_t {
	char message[255];
	char topic[255];
} message_t;
typedef struct sensor_t {
	int bus;
	int addr;
	int type;
	char* topic;
	QueueHandle_t messages;
    void (*tick)(struct sensor_t* sensor);
} sensor_t;

#ifdef ESP_PLATFORM // ESP32 (ESP-IDF)
	#define TASK_STACK_DEPTH 2048	// user task stack depth for ESP32
#else // ESP8266 (esp-open-rtos)
	#define TASK_STACK_DEPTH 256	// user task stack depth for ESP8266
#endif // ESP_PLATFORM
#define SENSOR_COUNT 5
#define I2CSensor_BUS 0
#define I2CSensor_SCL_PIN 5
#define I2CSensor_SDA_PIN 33
#define I2CSensor_FREQ I2C_FREQ_400K
extern uint8_t current_sensor;
void init_sensors(void);
void register_sensor(sensor_t* sensor);
extern sensor_t* sensors[SENSOR_COUNT];