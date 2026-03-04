#include "esp_mqtt.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "esp_at.h"

static bool g_mqtt_beschikbaar = false;

static esp_mqtt_bericht_callback_t g_callback = NULL;
static void *g_callback_user = NULL;

static char topic_buffer[128];
static char payload_buffer[512];

// URC parser: zoekt +MQTTSUBRECV regels en roept callback aan
static void urc_parser(const char *regel, void *user)
{
    (void)user;

    // Verwacht formaat (ESP-AT):
    // +MQTTSUBRECV:<id>,"topic",<len>,<data>

    if (strncmp(regel, "+MQTTSUBRECV:", 13) != 0)
        return;

    if (!g_callback)
        return;

    const char *p = regel + 13;

    // skip id tot comma
    while (*p && *p != ',') p++;
    if (*p != ',') return;
    p++;

    // topic tussen quotes
    const char *q1 = strchr(p, '"');
    if (!q1) return;
    const char *q2 = strchr(q1 + 1, '"');
    if (!q2) return;

    size_t topic_len = (size_t)(q2 - (q1 + 1));
    if (topic_len >= sizeof(topic_buffer)) topic_len = sizeof(topic_buffer) - 1;
    memcpy(topic_buffer, q1 + 1, topic_len);
    topic_buffer[topic_len] = 0;

    const char *na_topic = q2 + 1;
    if (*na_topic != ',') return;
    na_topic++;

    // len tot volgende comma
    const char *comma2 = strchr(na_topic, ',');
    if (!comma2) return;

    int payload_len = atoi(na_topic);
    const char *data = comma2 + 1;

    if (payload_len < 0) payload_len = 0;
    if (payload_len >= (int)sizeof(payload_buffer)) payload_len = (int)sizeof(payload_buffer) - 1;

    int beschikbaar = (int)strlen(data);
    int kopie_len = (payload_len <= beschikbaar) ? payload_len : beschikbaar;

    memcpy(payload_buffer, data, (size_t)kopie_len);
    payload_buffer[kopie_len] = 0;

    g_callback(topic_buffer, payload_buffer, kopie_len, g_callback_user);
}

esp_err_t esp_mqtt_detecteer(uint32_t timeout_ms)
{
    esp_err_t e = esp_at_cmd("AT+MQTTUSERCFG=?\r\n", "OK", timeout_ms);
    g_mqtt_beschikbaar = (e == ESP_OK);
    return g_mqtt_beschikbaar ? ESP_OK : ESP_ERR_NOT_SUPPORTED;
}

void esp_mqtt_set_bericht_callback(esp_mqtt_bericht_callback_t callback, void *gebruiker_data)
{
    g_callback = callback;
    g_callback_user = gebruiker_data;

    // Koppel onze parser aan de AT driver (URC regels)
    esp_at_set_urc_callback(urc_parser, gebruiker_data);
}

esp_err_t esp_mqtt_configureer(const esp_mqtt_config_t *config, uint32_t timeout_ms)
{
    if (!g_mqtt_beschikbaar) return ESP_ERR_NOT_SUPPORTED;
    if (!config || !config->client_id) return ESP_ERR_BAD_PARAM;

    const char *user = config->gebruikersnaam ? config->gebruikersnaam : "";
    const char *pass = config->wachtwoord ? config->wachtwoord : "";

    // ESP-AT heeft een "disable_clean_session" veld (0 = clean, 1 = niet clean)
    int disable_clean = config->schone_sessie ? 0 : 1;

    char cmd[256];
    snprintf(cmd, sizeof(cmd),
             "AT+MQTTUSERCFG=%d,1,\"%s\",\"%s\",\"%s\",0,%d,%d\r\n",
             config->verbinding_id,
             config->client_id,
             user,
             pass,
             config->keepalive_seconden,
             disable_clean);

    return esp_at_cmd(cmd, "OK", timeout_ms);
}

esp_err_t esp_mqtt_verbind(const char *host, int poort, uint32_t timeout_ms)
{
    if (!g_mqtt_beschikbaar) return ESP_ERR_NOT_SUPPORTED;
    if (!host || poort <= 0) return ESP_ERR_BAD_PARAM;

    char cmd[256];
    // laatste parameter is reconnect (0 = uit) in veel ESP-AT builds
    snprintf(cmd, sizeof(cmd), "AT+MQTTCONN=0,\"%s\",%d,0\r\n", host, poort);
    return esp_at_cmd(cmd, "OK", timeout_ms);
}

esp_err_t esp_mqtt_subscribe(const char *topic, int qos, uint32_t timeout_ms)
{
    if (!g_mqtt_beschikbaar) return ESP_ERR_NOT_SUPPORTED;
    if (!topic) return ESP_ERR_BAD_PARAM;

    if (qos < 0) qos = 0;
    if (qos > 2) qos = 2;

    char cmd[256];
    snprintf(cmd, sizeof(cmd), "AT+MQTTSUB=0,\"%s\",%d\r\n", topic, qos);
    return esp_at_cmd(cmd, "OK", timeout_ms);
}

esp_err_t esp_mqtt_unsubscribe(const char *topic, uint32_t timeout_ms)
{
    if (!g_mqtt_beschikbaar) return ESP_ERR_NOT_SUPPORTED;
    if (!topic) return ESP_ERR_BAD_PARAM;

    char cmd[256];
    snprintf(cmd, sizeof(cmd), "AT+MQTTUNSUB=0,\"%s\"\r\n", topic);
    return esp_at_cmd(cmd, "OK", timeout_ms);
}

esp_err_t esp_mqtt_publiceer(const char *topic,
                              const char *payload,
                              int qos,
                              bool retain,
                              uint32_t timeout_ms)
{
    if (!g_mqtt_beschikbaar) return ESP_ERR_NOT_SUPPORTED;
    if (!topic || !payload) return ESP_ERR_BAD_PARAM;

    if (qos < 0) qos = 0;
    if (qos > 2) qos = 2;

    int r = retain ? 1 : 0;

    // Let op: payload met quotes kan issues geven met AT strings.
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "AT+MQTTPUB=0,\"%s\",\"%s\",%d,%d\r\n",
             topic, payload, qos, r);

    return esp_at_cmd(cmd, "OK", timeout_ms);
}

esp_err_t esp_mqtt_disconnect(uint32_t timeout_ms)
{
    if (!g_mqtt_beschikbaar) return ESP_ERR_NOT_SUPPORTED;
    return esp_at_cmd("AT+MQTTDISCONN=0\r\n", "OK", timeout_ms);
}

// =============================
// MQTT publish via PUBRAW
// =============================
bool mqtt_pubraw(const char *topic, const char *payload)
{
    int len = (int)strlen(payload);

    char cmd[256];
    snprintf(cmd, sizeof(cmd), "AT+MQTTPUBRAW=0,\"%s\",%d,0,0\r\n", topic, len);
    printf("MQTT publish: AT+MQTTPUBRAW");
    
    printf("[INFO] CMD: %s", cmd);

    // wacht op '>' prompt
    if (esp_at_cmd(cmd, ">", 15000) != ESP_OK)
    {
        printf("geen '>' prompt (PUBRAW start mislukt)");
        return false;
    }

    // stuur exact payload bytes (geen CRLF!)
    esp_at_write(payload);

    // geef tijd voor +MQTTPUB:OK / +MQTTPUB:FAIL output
    drain_uart(1500);

    printf("PUBRAW verzonden (controleer +MQTTPUB:OK/FAIL in output)");
    return true;
}
