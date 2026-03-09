#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

#include "MQTT.h"
#include "Timo.h"

float gewicht;

void Message_Recieved(const char *topic, const char *payload){
    printf("\n[ONTVANGEN] Topic: %s | Bericht: %s\n", topic, payload);

    // receive message op topic
    if (strcmp(topic, "tap/snsr") == 0) {
        printf("Ontvangen payload: %s\n", payload);
        memccpy(&gewicht, payload, sizeof(payload));
        if (gewicht > 30 && gewicht < 200){
            MQTT_publish("tap/actr","open",0); //publish
        }
        else if (gewicht > 1000){
            MQTT_publish("tap/actr","dicht",0);
        }
    }

}

void Timo_loop(){

}

bool Timo_Init(){

    //Subscriben naar topics die je wilt ontvangen
    //LET OP!!!
    //iedereen ontvangt van alle topics dus filter goed!!!
    MQTT_subscribe("tap/snsr",0);

    //Callback instellen waar messages binnen komen
    if (set_callback(Message_Recieved)==false){
        return false;
    }

    return true;
}