// esp_at.h
#pragma once

#include <stdbool.h>
#include <stdint.h>

// Resultaatcodes voor AT-functies
typedef enum {
    ESP_OK = 0,
    ESP_ERR_TIMEOUT,
    ESP_ERR_ERROR,
    ESP_ERR_FAIL,
    ESP_ERR_BUSY,
    ESP_ERR_OVERFLOW,
    ESP_ERR_NOT_SUPPORTED,
    ESP_ERR_BAD_PARAM,
} esp_err_t;

// Callback type: wordt aangeroepen bij "URC" regels (asynchrone meldingen van ESP)
typedef void (*esp_urc_cb_t)(const char *regel, void *gebruiker_data);

// Configuratie voor UART verbinding naar ESP-AT module
typedef struct {
    uint32_t baudrate;   // bijv. 115200
    uint8_t  tx_pin;     // RP2350 TX -> ESP RX
    uint8_t  rx_pin;     // RP2350 RX <- ESP TX
    uint8_t  reset_pin;  // naar ESP EN/CHIP_EN of RESET (optioneel)
} esp_at_cfg_t;

// Initialiseer UART + buffers + interrupts
void esp_at_init(const esp_at_cfg_t *config);

// (optioneel) reset functie (in deze implementatie alleen flush)
esp_err_t esp_at_reset(uint32_t timeout_ms);

// Zet een callback voor URC regels
void esp_at_set_urc_callback(esp_urc_cb_t cb, void *gebruiker_data);

// Schrijf rechtstreeks bytes naar ESP UART
void esp_at_write(const char *tekst);

// Buffer leegmaken (RX ringbuffer + UART RX leegtrekken)
void esp_at_flush(void);

// Stuur een AT commando en wacht tot je een token ziet (bijv. "OK")
esp_err_t esp_at_cmd(const char *commando, const char *verwacht_token, uint32_t timeout_ms);

// Verwerk inkomende UART bytes: print naar stdout + parse regels + URC callback
void esp_at_poll(void);

// Laatste volledig gelezen regel (zonder CRLF)
const char* esp_at_last_line(void);
void drain_uart();

bool stuur_cmd_ok(const char *omschrijving, const char *cmd, uint32_t timeout_ms);
bool wacht_op_at(int timeout_ms);