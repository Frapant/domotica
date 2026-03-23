#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "scale.h"

// I2C defines
#define I2C_PORT i2c0
#define I2C_SDA 20
#define I2C_SCL 21
#define SCALE_ADDR 0x26

// addresen binnen scale en data buffer
uint8_t zero = 0x50; // register voor offset
uint8_t reg = 0x10; // register van het gewicht als float in gram
uint8_t buffer[4]; // buffer van de data

// globale variabelen voor gewicht en nulpunt en staat
float nulpunt;
float gewicht;

void weigh_init()
{
    // I2C Initialisation. Using it at 100Khz.
    i2c_init(I2C_PORT, 100*1000);
    
    // i2c pins
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    //1x nullen
    i2c_write_blocking(i2c0, SCALE_ADDR, &zero, 1, false);
    i2c_write_blocking(i2c0, SCALE_ADDR, &reg, 1, false); // register die we willen lezen / opvragen
    i2c_read_blocking(i2c0, SCALE_ADDR, buffer, 4, false);

    // sla 1x het nulpunt op
    memcpy(&nulpunt, buffer, 4);

    sleep_ms(10);
}

void weigh_loop()
{
        // altijd wegen
        i2c_write_blocking(i2c0, SCALE_ADDR, &reg, 1, false); // register die we willen lezen / opvragen
        i2c_read_blocking(i2c0, SCALE_ADDR, buffer, 4, false);  // check of alles wordt ontvangen en write gewicht naar de buffer

        // copy naar andere variable voor verwerking
        memcpy(&gewicht, buffer, 4);
}
