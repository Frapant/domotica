// bare minimum
#include <stdio.h>
#include "pico/stdlib.h"

// custom headers
#include "scale.h"
#include "motor.h"

// mashallah
int main()
{
    stdio_init_all();

    weigh_init();
    while (true) {
        // altijd gewicht checken
        weigh_loop();

        // voor monitor testing
        printf("Gewogen gewicht: %.2f\n", gewicht);
        sleep_ms(100);

        // kraan open bij gewicht en na 5 seconden dicht
        if (gewicht == 1000) {
            open();
            sleep_ms(5000);
            dicht();
        }
        
    }
}
