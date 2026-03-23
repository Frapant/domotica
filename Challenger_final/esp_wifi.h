#pragma once
#include "esp_at.h"
#include <stdint.h>

// Zet echo uit (ATE0)
esp_err_t wifi_echo_uit(void);

// Zet ESP in station mode
esp_err_t wifi_station_mode(void);

// Verbind met WiFi netwerk
esp_err_t wifi_verbind(const char *ssid,
                       const char *wachtwoord,
                       uint32_t timeout_ms);

// Verbreek WiFi verbinding
esp_err_t wifi_verbreek(void);

// Vraag IP-adres op
esp_err_t wifi_vraag_ip(uint32_t timeout_ms);
