#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#include "MQTT.h"
#include "Timo.h"
#include "sql.h"

// voor het toevoegen aan database
char *type = "gewicht";

// alle gewicht var
double gewicht = 0;         // live gewogen gewicht
double leeg_gewicht = 0;    // nul gewicht
double prev_gewicht = 0;    // live gewicht - 1 voor checken of t constant is
double start_gewicht = 0;   // begin gewicht zodra die opengaat
double log_gewicht = 0;     // gewicht dat naar de db wordt gezet aka verschil in start/live aka hoeveel ml er in t glas is gekomen

// bool states voor controle of kraan open of dicht is
bool kan_open = true;       // MAG de kraan open aka, staat er niks op?
bool is_open = false;       // of de kraan nu open is of niet
bool first = true;          // puur voor eerste message

// teller, om als pauze te gebruiken ipv sleep()
int i = 0;

// runt zodra message wordt ontvangen op de sub
void Message_Recieved_T(const char *topic, const char *payload){

    // als ontvangen topic dit is dan doe alles
    if (strcmp(topic, "tap/snsr") == 0) {   
        // convert  payload naar double voor voorwaarden (ontvangt alleen maar gewicht in juiste vorm)
        gewicht = atof(payload);

        // checkt of het het eerste ontvangen bericht is voor nulpunt
        if (first == true) {
            leeg_gewicht = gewicht;
            first = false;   
        }

        // kan kijken hiermee kijken of gewicht stabiel is
        prev_gewicht = gewicht;

        // meer gewicht dan leeg en of ie open mag en gewicht is 'stabiel'
        // +-0.3 is kleine marge want fluctueert heel klein beetje
        if (gewicht > leeg_gewicht + 10 && kan_open == true && (gewicht < prev_gewicht + 0.3 || gewicht > prev_gewicht - 0.3)){
            // verhoogt de teller totdat ie 100 is, dus gewicht moet voor bepaald aantal seconden stabiel zijn
            // na 50ms * i gaat open (publish interval op challenger is 50ms) 40 == 2s
            i++;
            if (i == 40){
                MQTT_publish("tap/actr","open",0);
                kan_open = false;
                is_open = true;
                start_gewicht = gewicht;
            }
        }

        // zodra huidig gewicht groter is dan start gewicht + 100gram ga dicht en log en reset de waarden   
        if (gewicht > start_gewicht + 100 && is_open == true){
            MQTT_publish("tap/actr","dicht",0);
            log_gewicht = gewicht - start_gewicht;
            insert_meting(log_gewicht, type);
            is_open = false;
            start_gewicht = 0;
            i = 0;
        }
        
        // als open en gewicht komt onder start waarde aka glas weg, ga dicht
        if (is_open == true && gewicht < start_gewicht) {
            MQTT_publish("tap/actr","dicht",0);
            is_open = false;
            start_gewicht = 0;
            i = 0;
        }

        // zorgt dat alleen open kan als de weegschaal leeg is
        if (gewicht < leeg_gewicht + 10) {
            kan_open =  true;
        }

        // debug string
        //printf("gewicht:%.2f || leeg:%.2f || start:%.2f || prev:%.2f\n", gewicht, leeg_gewicht, start_gewicht, prev_gewicht);
    }
}

void Timo_loop(){

}

bool Timo_Init(){
    // connect naar de database 1 maal bij opstarten
    db_connect();

    //Subscriben naar topics die je wilt ontvangen
    //LET OP!!!
    //iedereen ontvangt van alle topics dus filter goed!!!
    MQTT_subscribe("tap/snsr",0);

    //Callback instellen waar messages binnen komen
    if (set_callback(Message_Recieved_T)==false){
        return false;
    }

    return true;
}