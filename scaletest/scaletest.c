#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// I2C defines
// This example will use I2C0 on GPIO8 (SDA) and GPIO9 (SCL) running at 400KHz.
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define I2C_PORT i2c0
#define I2C_SDA 20
#define I2C_SCL 21
#define SCALE_ADDR 0x26

int main()
{
    stdio_init_all();

    // I2C Initialisation. Using it at 100Khz.
    i2c_init(I2C_PORT, 100*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    // For more examples of I2C use see https://github.com/raspberrypi/pico-examples/tree/master/i2c

    uint8_t zero = 0x50; // register voor offset
    uint8_t reg = 0x10; // register van het gewicht als float in gram
    uint8_t buffer[4]; // buffer van de data

    int r;
    float gewicht;

    i2c_write_blocking(i2c0, SCALE_ADDR, &zero, 1, false);

    sleep_ms(1000);

    while (true) {
        i2c_write_blocking(i2c0, SCALE_ADDR, reg, 1, true); // register die we willen lezen / opvragen
        i2c_read_blocking(i2c0, SCALE_ADDR, buffer, 4, false);  // check of alles wordt ontvangen en write gewicht naar de buffer

        //data van buffer, naar geheugen adress van gewicht kopieren en dan converten
        memcpy(&gewicht, buffer, 4);

        printf("Weight: %.2f g\n", gewicht);
        
        sleep_ms(100);
    }
}
