#pragma once
typedef struct sensor_t {
	int i2c_scl;
	int i2c_sda;
	int i2c_freq;
	int i2c_bus;
	int type;
	char* topic;
	char messages[10][255];
    void (*tick)(void);
	char* (*message)(void);
} sensor_t;
#define SENSOR_COUNT 2
extern sensor_t sensors[SENSOR_COUNT];