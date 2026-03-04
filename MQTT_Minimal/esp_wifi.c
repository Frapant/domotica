#include "esp_wifi.h"
#include <stdio.h>

esp_err_t wifi_echo_uit(void)
{
    return esp_at_cmd("ATE0\r\n", "OK", 1000);
}

esp_err_t wifi_station_mode(void)
{
    return esp_at_cmd("AT+CWMODE=1\r\n", "OK", 2000);
}

esp_err_t wifi_verbind(const char *ssid, const char *wachtwoord, uint32_t timeout_ms)
{
    if (!ssid || !wachtwoord) return ESP_ERR_BAD_PARAM;

    char commando[256];
    snprintf(commando, sizeof(commando),
             "AT+CWJAP=\"%s\",\"%s\"\r\n",
             ssid, wachtwoord);

    return esp_at_cmd(commando, "OK", timeout_ms);
}

esp_err_t wifi_verbreek(void)
{
    return esp_at_cmd("AT+CWQAP\r\n", "OK", 3000);
}

esp_err_t wifi_vraag_ip(uint32_t timeout_ms)
{
    return esp_at_cmd("AT+CIFSR\r\n", "OK", timeout_ms);
}
