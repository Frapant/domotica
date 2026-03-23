#pragma once

#include "esp_at.h"
#include <stdint.h>
#include <stdbool.h>

// ============================================================
// MQTT configuratiestruct
// ============================================================
//
// Wordt gebruikt bij esp_mqtt_configureer().
// Dit beschrijft hoe de MQTT client zich gedraagt.
//

typedef struct {
    int         verbinding_id;     // meestal 0
    const char *client_id;         // unieke naam van deze client
    const char *gebruikersnaam;    // NULL toegestaan
    const char *wachtwoord;        // NULL toegestaan
    int         keepalive_seconden;
    bool        schone_sessie;     // true = clean session
} esp_mqtt_config_t;


// ============================================================
// Callback type voor ontvangen MQTT berichten
// ============================================================
//
// Wordt aangeroepen wanneer ESP-AT een
// +MQTTSUBRECV bericht ontvangt.
//

typedef void (*esp_mqtt_bericht_callback_t)(
    const char *topic,
    const char *payload,
    int payload_lengte,
    void *gebruiker_data);


// ============================================================
// Callback instellen voor ontvangen berichten
// ============================================================

void esp_mqtt_set_bericht_callback(
    esp_mqtt_bericht_callback_t callback,
    void *gebruiker_data);


// ============================================================
// MQTT configureren (AT+MQTTUSERCFG)
// ============================================================

esp_err_t esp_mqtt_configureer(
    const esp_mqtt_config_t *config,
    uint32_t timeout_ms);


// ============================================================
// Verbinden met MQTT broker (AT+MQTTCONN)
// ============================================================

esp_err_t esp_mqtt_verbind(
    const char *host,
    int poort,
    uint32_t timeout_ms);


// ============================================================
// Subscribe op topic
// ============================================================

bool esp_mqtt_subscribe(
    const char *topic,
    int qos,
    uint32_t timeout_ms);



// ============================================================
// Unsubscribe van topic
// ============================================================

esp_err_t esp_mqtt_unsubscribe(
    const char *topic,
    uint32_t timeout_ms);


// ============================================================
// Publiceer bericht
// ============================================================

esp_err_t esp_mqtt_publiceer(const char *topic,
                              const char *payload,
                              int qos,
                              bool retain,
                              uint32_t timeout_ms);

bool mqtt_pubraw(const char *topic, const char *payload);



// ============================================================
// Verbinding sluiten
// ============================================================

esp_err_t esp_mqtt_disconnect(uint32_t timeout_ms);