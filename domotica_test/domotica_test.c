#include <stdio.h>
#include "pico/stdlib.h"

//in1 = 10, in2 = 9 voor motor controller
#define IN1 2 
#define IN2 25 

//button pin = 13
#define BTN 7


int main()
{
    stdio_init_all();

    gpio_init(IN1);
    gpio_set_dir(IN1, GPIO_OUT);

    gpio_init(IN2);
    gpio_set_dir(IN2, GPIO_OUT);

    gpio_init(BTN);
    gpio_set_dir(BTN, GPIO_IN);
    gpio_pull_up(BTN);

    //hbrug testing
    while (true) {
        /*
        //open
        printf("open!\n");
        gpio_put(IN1, 1);
        gpio_put(IN2, 0);
        sleep_ms(30);
        gpio_put(IN1, 0);

        //dicht
        printf("dicht!\n");
        gpio_put(IN1, 0);
        gpio_put(IN2, 1);
        sleep_ms(30);
        gpio_put(IN2, 0);
              
        printf("hihi");
        gpio_put(IN1, 1);
        gpio_put(IN2, 0);
        sleep_ms(5000);
        gpio_put(IN1, 0);
        gpio_put(IN2, 1);
        sleep_ms(5000); 
        */ 
        while(gpio_get(BTN) == 0) {
            printf("hihi");
            gpio_put(IN1, 1);
            gpio_put(IN2, 0);
        }
        gpio_put(IN1, 0);
    } 
}
    
