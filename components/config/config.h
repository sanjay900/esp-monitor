typedef struct{
	char f_version[4];
	char h_version[4];
	int id;
	char name[20];
	char location[20];
	char sensor_type[4];
	int refresh_rate;
	int alarm_treshold_min;
	int alarm_treshold_max;
	char esp32_ip[15];
	char mqtt_ip[30];
	int log_activ;
	int log_days;
} config_struct;

static config_struct config_data;
void init_config(void);