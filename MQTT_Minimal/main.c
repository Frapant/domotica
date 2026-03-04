// main.c
// RP2350 + ESP32-C6 (ESP-AT) hardcoded WiFi + MQTT demo)

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "esp_at.h"
#include "esp_mqtt.h"

#define WIFI_SSID       "Domoticz"
#define WIFI_WACHTWOORD "wacthwoordofzo"

#define MQTT_BROKER_IP  "10.42.0.1"
#define MQTT_POORT      1883
#define MQTT_CLIENT_ID  "Datalogger01"
#define MQTT_SCHEME     1   // 1 = MQTT over TCP (geen TLS)

#define MQTT_TOPIC_SUB  "Makerspace/Logger01"
#define MQTT_TOPIC_PUB  "Makerspace/Logger01"

#define PUB_INTERVAL_MS 5000

// =============================
// main
// =============================



int main()
{
    stdio_init_all();
    sleep_ms(2000);

    printf("[INFO] RP2350 <-> ESP32-C6\r\n");

    // UART configuratie naar ESP
    esp_at_cfg_t configuratie = {
        .baudrate  = 115200,
        .tx_pin    = 4,
        .rx_pin    = 5,
        .reset_pin = 15
    };

    printf("[INFO] ESP-AT initialiseren\r\n");
    esp_at_init(&configuratie);

    printf("[INFO] Wachten op AT antwoord\r\n");
    if (!wacht_op_at(6000))
    {
        printf("[FOUT] Geen AT response. Controleer TX/RX pins en baudrate.\r\n");
        while (1) { esp_at_poll(); sleep_ms(10); }
    }
    printf("[OK] ESP reageert op AT\r\n");

    (void)stuur_cmd_ok("Echo uitzetten (ATE0)", "ATE0\r\n", 1500);

    // =============================
    // WiFi
    // =============================
    printf("\r\n[INFO] WiFi verbinden (hardcoded AT)\r\n");

    if (!stuur_cmd_ok("WiFi modus station: AT+CWMODE=1", "AT+CWMODE=1\r\n", 3000))
        while (1) { esp_at_poll(); sleep_ms(10); }

    {
        char cmd[256];
        snprintf(cmd, sizeof(cmd),
                 "AT+CWJAP=\"%s\",\"%s\"\r\n",
                 WIFI_SSID, WIFI_WACHTWOORD);

        if (!stuur_cmd_ok("WiFi verbinden: AT+CWJAP", cmd, 20000))
            while (1) { esp_at_poll(); sleep_ms(10); }
    }

    printf("\r\n[INFO] IP adres opvragen: AT+CIFSR\r\n");
    (void)esp_at_cmd("AT+CIFSR\r\n", "OK", 6000);
    drain_uart(200);

    // =============================
    // MQTT
    // =============================
    printf("\r\n[INFO] MQTT verbinden (hardcoded AT)\r\n");

    {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "AT+MQTTUSERCFG=0,%d,\"%s\",\"\",\"\",0,0,\"\"\r\n", MQTT_SCHEME, MQTT_CLIENT_ID);

        if (!stuur_cmd_ok("MQTT usercfg: AT+MQTTUSERCFG", cmd, 6000))
            while (1) { esp_at_poll(); sleep_ms(10); }
    }

    {
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "AT+MQTTCONN=0,\"%s\",%d,0\r\n", MQTT_BROKER_IP, MQTT_POORT);

        if (!stuur_cmd_ok("MQTT connect: AT+MQTTCONN", cmd, 12000))
            while (1) { esp_at_poll(); sleep_ms(10); }
    }

    {
        char cmd[320];
        snprintf(cmd, sizeof(cmd), "AT+MQTTSUB=0,\"%s\",0\r\n", MQTT_TOPIC_SUB);

        (void)stuur_cmd_ok("MQTT subscribe: AT+MQTTSUB", cmd, 8000);
    }

    printf("\r\n[INFO] MQTT status: AT+MQTTCONN?\r\n");
    esp_at_write("AT+MQTTCONN?\r\n");
    drain_uart(2000);

    printf("\r\n[INFO] MQTT subscribe lijst: AT+MQTTSUB?\r\n");
    esp_at_write("AT+MQTTSUB?\r\n");
    drain_uart(2000);

    // =============================
    // Runtime: publish elke 5 sec
    // =============================
    printf("\r\n[INFO] Runtime\r\n");
    printf("[INFO] Auto publish elke 5 seconden is AAN\r\n");
    printf("[INFO] Pub topic: %s\r\n", MQTT_TOPIC_PUB);
    printf("[INFO] Sub topic: %s\r\n", MQTT_TOPIC_SUB);

    int teller = 0;
    absolute_time_t volgende = make_timeout_time_ms(PUB_INTERVAL_MS);

    while (1)
    {
        esp_at_poll();

        if (time_reached(volgende))
        {
            char payload[128];
            snprintf(payload, sizeof(payload),
                     "{\"bron\":\"rp2350\",\"teller\":%d}", teller++);

            if(mqtt_pubraw(MQTT_TOPIC_PUB, payload)== true)
            {
                printf("[INFO] MQTT PUBRAW verzonden: %s\r\n", payload);
            }
            else
            {
                printf("[FOUT] MQTT PUBRAW mislukt\r\n");
            }

            volgende = make_timeout_time_ms(PUB_INTERVAL_MS);
        }

        sleep_ms(2);
    }
}