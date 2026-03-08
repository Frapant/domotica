#include <stdio.h>
#include "pico/stdlib.h"

// gpio pin nummers 
#define IN1 25 // fysiek 9
#define IN2 2 // fysiek 10

void init() {
    // H-brug kanalen
    gpio_init(IN1);
    gpio_set_dir(IN1, GPIO_OUT);

    gpio_init(IN2);
    gpio_set_dir(IN2, GPIO_OUT);

    gpio_put(IN1, 0);
    gpio_put(IN2, 0);
}

void open() {
    init();
    gpio_put(IN1, 1);
    sleep_ms(20);
    gpio_put(IN1, 0);
}

void dicht() {
    init();
    gpio_put(IN2, 1);
    sleep_ms(20);
    gpio_put(IN2, 0);
}
