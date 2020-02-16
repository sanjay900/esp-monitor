#pragma once
#define MAX_SENSOR_COUNT 5
typedef struct{
	char f_version[4];
	char h_version[4];
	int id;
	char name[20];
	char location[20];
	char sensors[10][MAX_SENSOR_COUNT];
	int sensor_count;
	int refresh_rate;
	char mqtt_ip[64];
} config_struct;

extern config_struct config_data;
void init_config(void);