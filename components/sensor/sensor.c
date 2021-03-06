#include "sensor.h"
#include "esp_log.h"
sensor_t* sensors[MAX_SENSOR_COUNT];
uint8_t current_sensor;

void register_sensor(sensor_t* sensor) {
	sensor->messages = xQueueCreate(MAX_MESSAGE_COUNT, sizeof(message_t));
    sensors[current_sensor++] = sensor;
}
void sensor_read(void *pvParameters) {
	TickType_t last_wakeup = xTaskGetTickCount();
	while (1) {
		for (int i =0; i < current_sensor; i++) {
			sensors[i]->tick(sensors[i]);
		}
		vTaskDelayUntil(&last_wakeup, config_data.refresh_rate / portTICK_PERIOD_MS);
	}
}
void init_sensor_read_task() {
	xTaskCreatePinnedToCore(sensor_read, "SENSOR_READ_TASK", 4096, NULL, 2,  NULL, 0);	//xTaskCreatePinnedToCore
}
