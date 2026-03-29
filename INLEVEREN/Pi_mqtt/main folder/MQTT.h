#ifndef _MQTT_H_
#define _MQTT_H_

#include <stdbool.h>

int MQTT_init();

void MQTT_publish(char topic[],char payload[], int qos);

void MQTT_subscribe(char topic[],int qos);

typedef void (*MQTT_CALLBACK)(const char *topic, const char *payload);

bool set_callback(MQTT_CALLBACK function);


#endif