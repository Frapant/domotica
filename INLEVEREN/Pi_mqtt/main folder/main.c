#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <mosquitto.h>

#include "MQTT.h"
#include "reamo/Reamon.h"
#include "frap/Timo.h"
#include "silent-ghost/Gijs.h"

int main() {

    if (MQTT_init()==0){
        printf("Systeem gestart. Ik stuur elke seconde een getal...\n");
    }
    else{
        printf("Mosquitto failed to start");
    }

    if (Reamon_Init()){
        printf("Reamon init succesvol\n");
    }
    else {
        printf("Reamon init niet succesvol\n");
    }

    if (Timo_Init()){
        printf("Timo init succesvol\n");
    }
    else{
        printf("Timo init niet succesvol\n");
    }
    if (Gijs_Init())
    {
        printf("Gijs init succesvol\n");
    }
    else
    {
        printf("Gijs init niet succesvol\n");
    }

    while(1) {
        Reamon_loop();
        Timo_loop();
        Gijs_loop();
        //printf("alive\n");
        sleep(5); // Wacht 1 seconde voor de volgende tel
    }
    return 0;
}