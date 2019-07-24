#include "mqtt_client.h"
void init_mqtt(mqtt_event_callback_t event_handler, const char *uri);
void publish(const char *data);