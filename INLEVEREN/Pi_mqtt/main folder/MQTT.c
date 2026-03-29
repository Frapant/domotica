#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <mosquitto.h>
#include <stdbool.h>

#define PI_IP "0.0.0.0"

#include "MQTT.h"

struct mosquitto *mosq;


MQTT_CALLBACK Callback1 = NULL;
MQTT_CALLBACK Callback2 = NULL;
MQTT_CALLBACK Callback3 = NULL;

bool set_callback(MQTT_CALLBACK function){
    if (!Callback1) Callback1 = function;
    else if (!Callback2) Callback2 = function;
    else if (!Callback3) Callback3 = function;
    else return false;
    return true;
}

void on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg) {
    char *topic = msg->topic;
    char *payload = (char *)msg->payload;

    (void)mosq;
    (void)obj;

    if (Callback1) Callback1(topic,payload);
    if (Callback2) Callback2(topic,payload);
    if (Callback3) Callback3(topic,payload);
}

void MQTT_subscribe(char topic[],int qos){
    mosquitto_subscribe(mosq, NULL, topic, qos);

}

void MQTT_publish(char topic[],char payload[], int qos){
    mosquitto_publish(mosq, NULL, topic, (int)strlen(payload), payload, qos, false);
}

int MQTT_init(){
    mosquitto_lib_init();

    mosq = mosquitto_new("teller_client2", false, NULL);
    if(!mosq) return 1;

    // Callback instellen voor inkomende berichten
    mosquitto_message_callback_set(mosq, on_message);

    if(mosquitto_connect(mosq, PI_IP, 1883, 60) != MOSQ_ERR_SUCCESS) {
        printf("Kan niet verbinden met broker.\n");
        return 1;
    }

    mosquitto_loop_start(mosq);

    return 0;

}