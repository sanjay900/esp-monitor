#pragma once
typedef struct{
	char f_version[4];
	char h_version[4];
	int id;
	char name[20];
	char location[20];
	char sensors[10][4];
	int sensor_count;
	int refresh_rate;
	int alarm_treshold_min;
	int alarm_treshold_max;
	char esp32_ip[15];
	char mqtt_ip[64];
	int log_activ;
	int log_days;
} config_struct;

extern config_struct config_data;
void init_config(void);