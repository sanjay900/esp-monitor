#pragma once
#include "mqtt_client.h"
#include "esp_system.h"
#include "config.h"
#include "esp_log.h"
#include "sensor.h"
void mqtt_init(void);
void publish(float temperature, float humidity);
extern void (*on_message)(message_t* msg);