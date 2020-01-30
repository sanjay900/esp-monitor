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
#define MAX_MESSAGE_COUNT 10
#define MAX_SENSOR_COUNT 5

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

extern uint8_t current_sensor;
void init_sensor_read_task(void);
void register_sensor(sensor_t* sensor);
extern sensor_t* sensors[MAX_SENSOR_COUNT];