// esp_at.c
// UART driver + ringbuffer + eenvoudige AT parser
// RP2350 <=> ESP32-C6 (ESP-AT firmware)

#include "esp_at.h"

#include "MQTT.h"

#include "pico/stdlib.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"

#include <string.h>
#include <stdio.h>

#define ESP_UART      uart1
#define ESP_UART_IRQ  UART1_IRQ

#ifndef ESP_RX_BUF_SIZE
#define ESP_RX_BUF_SIZE 2048
#endif

// ============================================================
// Ringbuffers
// ============================================================
//
// rx_buffer:
//   - alle bytes van ESP
//   - wordt gebruikt om regels te parsen
//
// relay_buffer:
//   - kopie van alle binnenkomende bytes
//   - wordt direct doorgestuurd naar stdout
//   - zodat je live ESP output ziet
//

static volatile uint16_t rx_head = 0;
static volatile uint16_t rx_tail = 0;
static uint8_t rx_buffer[ESP_RX_BUF_SIZE];

static volatile uint16_t relay_head = 0;
static volatile uint16_t relay_tail = 0;
static uint8_t relay_buffer[ESP_RX_BUF_SIZE];

static esp_at_cfg_t g_config;

static esp_urc_cb_t g_urc_callback = NULL;
static void *g_urc_user = NULL;

static char g_laatste_regel[256];


// ============================================================
// Ringbuffer helpers
// ============================================================

static inline void rx_push(uint8_t byte)
{
    uint16_t next = (rx_head + 1) % ESP_RX_BUF_SIZE;

    // Buffer vol → oudste byte weggooien
    if (next == rx_tail) {
        rx_tail = (rx_tail + 1) % ESP_RX_BUF_SIZE;
    }

    rx_buffer[rx_head] = byte;
    rx_head = next;
}

static inline int rx_pop(void)
{
    if (rx_head == rx_tail)
        return -1;  // leeg

    uint8_t byte = rx_buffer[rx_tail];
    rx_tail = (rx_tail + 1) % ESP_RX_BUF_SIZE;
    return (int)byte;
}

static inline void relay_push(uint8_t byte)
{
    uint16_t next = (relay_head + 1) % ESP_RX_BUF_SIZE;

    if (next == relay_tail) {
        relay_tail = (relay_tail + 1) % ESP_RX_BUF_SIZE;
    }

    relay_buffer[relay_head] = byte;
    relay_head = next;
}

static inline int relay_pop(void)
{
    if (relay_head == relay_tail)
        return -1;

    uint8_t byte = relay_buffer[relay_tail];
    relay_tail = (relay_tail + 1) % ESP_RX_BUF_SIZE;
    return (int)byte;
}


// ============================================================
// UART interrupt handler
// ============================================================
//
// Wordt automatisch aangeroepen wanneer er data binnenkomt.
// Leest alle beschikbare bytes uit de UART FIFO.
//

static void esp_uart_isr(void)
{
    while (uart_is_readable(ESP_UART)) {
        uint8_t byte = (uint8_t)uart_getc(ESP_UART);
        rx_push(byte);
        relay_push(byte);
    }
}


// ============================================================
// Publieke functies
// ============================================================

void esp_at_set_urc_callback(esp_urc_cb_t cb, void *user)
{
    g_urc_callback = cb;
    g_urc_user = user;
}

void esp_at_init(const esp_at_cfg_t *config)
{
    g_config = *config;

    // Voor betrouwbaarheid: vaste baudrate
    g_config.baudrate = 115200;

    // Reset pin initialiseren (indien aangesloten)
    if (g_config.reset_pin != 0xFF) {
        gpio_init(g_config.reset_pin);
        gpio_set_dir(g_config.reset_pin, GPIO_OUT);
        gpio_put(g_config.reset_pin, 1); // enable
    }

    // UART configureren
    uart_init(ESP_UART, g_config.baudrate);
    gpio_set_function(g_config.tx_pin, GPIO_FUNC_UART);
    gpio_set_function(g_config.rx_pin, GPIO_FUNC_UART);

    uart_set_format(ESP_UART, 8, 1, UART_PARITY_NONE);
    uart_set_fifo_enabled(ESP_UART, true);

    // Interrupt activeren
    irq_set_exclusive_handler(ESP_UART_IRQ, esp_uart_isr);
    irq_set_enabled(ESP_UART_IRQ, true);
    uart_set_irq_enables(ESP_UART, true, false);

    esp_at_flush();

    DEBUG_PRINT("[INFO] ESP resetten\r\n");
    esp_at_reset(g_config.reset_pin);

    sleep_ms(3000);  // ESP tijd geven om te booten
}

void esp_at_flush(void)
{
    rx_head = rx_tail = 0;
    relay_head = relay_tail = 0;

    while (uart_is_readable(ESP_UART)) {
        (void)uart_getc(ESP_UART);
    }

    g_laatste_regel[0] = '\0';
}

void esp_at_write(const char *tekst)
{
    uart_puts(ESP_UART, tekst);
}

const char* esp_at_last_line(void)
{
    return g_laatste_regel;
}


// ============================================================
// Regel parser
// ============================================================
//
// Leest één tekstregel (zonder CR/LF)
//

static bool lees_regel(char *uit, size_t max, uint32_t timeout_ms)
{
    uint32_t start = to_ms_since_boot(get_absolute_time());
    size_t i = 0;

    // Eerste niet-CR/LF karakter zoeken
    while (to_ms_since_boot(get_absolute_time()) - start < timeout_ms) {
        int c = rx_pop();
        if (c < 0) { tight_loop_contents(); continue; }

        char ch = (char)c;
        if (ch == '\r' || ch == '\n') continue;

        uit[i++] = ch;
        break;
    }

    if (i == 0) return false;

    while (to_ms_since_boot(get_absolute_time()) - start < timeout_ms) {
        int c = rx_pop();
        if (c < 0) { tight_loop_contents(); continue; }

        char ch = (char)c;

        if (ch == '\r') continue;

        if (ch == '\n') {
            uit[i] = '\0';
            return true;
        }

        if (i < max - 1)
            uit[i++] = ch;
        else {
            uit[i] = '\0';
            return true;
        }
    }

    uit[i] = '\0';
    return true;
}


// ============================================================
// Poll functie
// ============================================================
//
// 1) Print alle binnenkomende bytes direct
// 2) Parse regels en roep callback aan
//
/*
void esp_at_poll(void)
{
    int byte;

    // Live output
    while ((byte = relay_pop()) >= 0) {
        //putchar((char)byte);
    }

    // Regels verwerken
    char regel[256];

    while (lees_regel(regel, sizeof(regel), 1)) {

        strncpy(g_laatste_regel, regel, sizeof(g_laatste_regel) - 1);
        g_laatste_regel[sizeof(g_laatste_regel) - 1] = '\0';

        if (g_urc_callback){
            g_urc_callback(regel, g_urc_user);
        }
    }
}
*/

// These must be static so they remember their state between function calls
static char parse_buf[256];
static size_t parse_idx = 0;

void esp_at_poll(void)
{
    // 1. Handle the Live Output (Relay)
    int b;
    while ((b = relay_pop()) >= 0) {
        if (DEBUG==true){
            putchar((char)b);
        }
    }

    // 2. Handle the Logic (RX) - Non-blocking State Machine
    int c;
    while ((c = rx_pop()) >= 0) {
        char ch = (char)c;

        if (ch == '\r') continue; // Ignore Carriage Return

        if (ch == '\n') {
            // We found a full line!
            if (parse_idx > 0) {
                parse_buf[parse_idx] = '\0';

                // Copy to global storage
                strncpy(g_laatste_regel, parse_buf, sizeof(g_laatste_regel) - 1);
                g_laatste_regel[sizeof(g_laatste_regel) - 1] = '\0';

                // Fire the callback
                if (g_urc_callback) {
                    g_urc_callback(parse_buf, g_urc_user);
                }
                
                parse_idx = 0; // Reset buffer for next line
            }
        } 
        else {
            // Add character to buffer if there is space
            if (parse_idx < sizeof(parse_buf) - 1) {
                parse_buf[parse_idx++] = ch;
            }
        }
    }
}

// ============================================================
// AT commando uitvoeren
// ============================================================

static bool is_eind_token(const char *regel)
{
    if (strcmp(regel, "OK") == 0) return true;
    if (strstr(regel, "ERROR")) return true;
    if (strstr(regel, "FAIL")) return true;
    if (strstr(regel, "busy")) return true;
    return false;
}

esp_err_t esp_at_cmd(const char *commando,
                     const char *verwacht_token,
                     uint32_t timeout_ms)
{
    if (!commando || !verwacht_token)
        return ESP_ERR_BAD_PARAM;

    esp_at_flush();
    esp_at_write(commando);

    uint32_t start = to_ms_since_boot(get_absolute_time());
    char regel[256];

    while (to_ms_since_boot(get_absolute_time()) - start < timeout_ms) {

        if (!lees_regel(regel, sizeof(regel), 50))
            continue;

        strncpy(g_laatste_regel, regel, sizeof(g_laatste_regel) - 1);
        g_laatste_regel[sizeof(g_laatste_regel) - 1] = '\0';

        if (g_urc_callback &&
            !is_eind_token(regel) &&
            !strstr(regel, verwacht_token))
        {
            g_urc_callback(regel, g_urc_user);
        }

        if (strstr(regel, "busy"))  return ESP_ERR_BUSY;
        if (strstr(regel, "FAIL"))  return ESP_ERR_FAIL;
        if (strstr(regel, "ERROR")) return ESP_ERR_ERROR;

        if (strstr(regel, verwacht_token))
            return ESP_OK;
    }

    return ESP_ERR_TIMEOUT;
}

esp_err_t esp_at_reset(uint32_t timeout_ms)
{
    (void)timeout_ms;
    esp_at_flush();
    return ESP_OK;
}

// =============================
// ESP hard reset
// =============================
void esp_harde_reset(uint reset_pin)
{
    if (reset_pin == 0xFF || reset_pin == (uint)-1) return;

    gpio_init(reset_pin);
    gpio_set_dir(reset_pin, GPIO_OUT);

    gpio_put(reset_pin, 0);
    sleep_ms(120);
    gpio_put(reset_pin, 1);
    sleep_ms(500);
}


// =============================
// Pomp alleen UART/URC door
// =============================
void drain_uart(uint32_t ms)
{
    absolute_time_t tot = make_timeout_time_ms(ms);
    while (!time_reached(tot))
    {
        esp_at_poll();
        sleep_ms(2);
    }
}


// =============================
// Commando sturen + loggen
// =============================
bool stuur_cmd_ok(const char *omschrijving, const char *cmd, uint32_t timeout_ms)
{
    DEBUG_PRINT("\r\n[INFO] %s\r\n", omschrijving);
    DEBUG_PRINT("[INFO] CMD: %s", cmd);

    esp_err_t r = esp_at_cmd(cmd, "OK", timeout_ms);
    if (r == ESP_OK)
    {
        DEBUG_PRINT("[OK]   OK\r\n");
        return true;
    }

    printf("[FOUT] mislukt (zie ERROR/FAIL in output)\r\n");
    return false;
}

// =============================
// Wacht tot ESP reageert op AT
// =============================
bool wacht_op_at(int timeout_ms)
{
    absolute_time_t eindtijd = make_timeout_time_ms(timeout_ms);

    while (!time_reached(eindtijd))
    {
        esp_at_poll();
        if (esp_at_cmd("AT\r\n", "OK", 300) == ESP_OK) return true;
        sleep_ms(50);
    }
    return false;
}