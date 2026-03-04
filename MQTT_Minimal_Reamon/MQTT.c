// main.c
// RP2350 + ESP32-C6 (ESP-AT) hardcoded WiFi + MQTT demo)

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "MQTT.h"
#include "esp_at.h"
#include "esp_mqtt.h"
#include "esp_wifi.h"

#define WIFI_SSID       "TRG"
#define WIFI_WACHTWOORD "12345678"

#define MQTT_BROKER_IP  "10.42.0.1"
#define MQTT_POORT      1883
#define MQTT_CLIENT_ID  "Datalogger01"
#define MQTT_SCHEME     1   // 1 = MQTT over TCP (geen TLS)

#define MQTT_TOPIC_SUB  "Makerspace/Logger01"
#define MQTT_TOPIC_PUB  "Makerspace/Logger01"

#define PUB_INTERVAL_MS 5000

mqtt_message_sent_function FeedBackFunctionSent = NULL;
mqtt_message_recieved_function FeedBackFunctionRecieve = NULL;

esp_mqtt_config_t mqtt_cfg = {
    .verbinding_id       = 0,                // Meestal 0, tenzij je meerdere verbindingen hebt
    .client_id           = MQTT_CLIENT_ID, // Unieke ID voor de broker
    .gebruikersnaam      = "",  // Je MQTT gebruikersnaam (of NULL)
    .wachtwoord          = "", // Je MQTT wachtwoord (of NULL)
    .keepalive_seconden  = 60,               // Standaard hartslag van de verbinding
    .schone_sessie       = true              // 'true' verwijdert oude data bij herstart
};

// =============================
// Bericht ontvangen
// =============================

bool Print_Output = true;

void setMQTToutput(bool Output){
    Print_Output = Output;
}

void setFeedBackSent(mqtt_message_sent_function function){
    FeedBackFunctionSent=function;
}

void setFeedBackRecieve(mqtt_message_recieved_function function){
    FeedBackFunctionRecieve=function;
}

void MQTT_get_message(const char *topic, const char *payload, int len, void *user) {
    if (Print_Output==true){
        printf("\r\n--- MQTT BERICHT ONTVANGEN ---\r\n");
        printf("Topic:   %s\r\n", topic);
        printf("Payload: %s\r\n", payload);
        printf("------------------------------\r\n");
    }
    if (FeedBackFunctionRecieve){
        FeedBackFunctionRecieve(topic,payload);
    }
}

void MQTT_send_message(const char *topic, const char *payload) {
    if (Print_Output==true){
        printf("\r\n--- MQTT BERICHT VERZONDEN ---\r\n");
        printf("Topic:   %s\r\n", topic);
        printf("Payload: %s\r\n", payload);
        printf("------------------------------\r\n");
    }
    if (FeedBackFunctionSent){
        FeedBackFunctionSent(topic,payload);
    }
}


// =============================
// Initialiseren
// =============================

bool mqtt_init(){
    DEBUG_PRINT("[INFO] RP2350 <-> ESP32-C6\r\n");

    // UART configuratie naar ESP
    esp_at_cfg_t configuratie = {
        .baudrate  = 115200,
        .tx_pin    = 4,
        .rx_pin    = 5,
        .reset_pin = 15
    };

    DEBUG_PRINT("[INFO] ESP-AT initialiseren\r\n");
    esp_at_init(&configuratie);

    //functie waar data heen moet van de topics waar je op ben gesubscibed
    esp_mqtt_set_bericht_callback(MQTT_get_message,"iets waarom is dit nodig geen idee!!");


    DEBUG_PRINT("[INFO] Wachten op AT antwoord\r\n");
    if (!wacht_op_at(6000))
    {
        printf("[FOUT] Geen AT response. Controleer TX/RX pins en baudrate.\r\n");
        return false;
    }
    DEBUG_PRINT("[OK] ESP reageert op AT\r\n");

    (void)stuur_cmd_ok("Echo uitzetten (ATE0)", "ATE0\r\n", 1500);

    // =============================
    // WiFi
    // =============================
    DEBUG_PRINT("\r\n[INFO] WiFi verbinden (hardcoded AT)\r\n");

    if (!stuur_cmd_ok("WiFi modus station: AT+CWMODE=1", "AT+CWMODE=1\r\n", 3000)){
        printf("[FOUT] Geen Wifi response. Controleer TX/RX pins en baudrate.\r\n");
        return false;
    }
    
    DEBUG_PRINT("\r\n[INFO] Verbinden met netwerk\r\n");
    wifi_verbind(WIFI_SSID,WIFI_WACHTWOORD,20000);
    

    DEBUG_PRINT("\r\n[INFO] IP adres opvragen: AT+CIFSR\r\n");
    wifi_vraag_ip(6000);
    drain_uart(200);
    sleep_ms(1000);

    // =============================
    // MQTT
    // =============================
    DEBUG_PRINT("\r\n[INFO] MQTT verbinden (hardcoded AT)\r\n");

    esp_mqtt_configureer(&mqtt_cfg,6000);
    
    esp_mqtt_verbind(MQTT_BROKER_IP,MQTT_POORT,12000);

    esp_mqtt_subscribe(MQTT_TOPIC_SUB,0,8000);

    DEBUG_PRINT("\r\n[INFO] MQTT status: AT+MQTTCONN?\r\n");
    esp_at_write("AT+MQTTCONN?\r\n");
    drain_uart(2000);

    DEBUG_PRINT("\r\n[INFO] MQTT subscribe lijst: AT+MQTTSUB?\r\n");
    esp_at_write("AT+MQTTSUB?\r\n");
    drain_uart(2000);
}

// =============================
// Bericht verzenden
// =============================

bool mqtt_send(char data[], char topic[]){
    if(mqtt_pubraw(topic, data)== true)
    {
        DEBUG_PRINT("[INFO] MQTT PUBRAW verzonden: %s\r\n", data);
        MQTT_send_message(topic, data);
        return true;
    }
    else
    {
        printf("[FOUT] MQTT PUBRAW mislukt\r\n");
        return false;
    }

}

// =============================
// Subscriben
// =============================

// =============================
// Syc
// =============================

void SyncMqttData(){
    esp_at_poll();
}
