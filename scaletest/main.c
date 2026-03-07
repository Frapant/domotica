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
        weigh_loop();

        // voor monitor testing
        printf("Gewogen gewicht: %.2f\n", gewicht);
        sleep_ms(100);


        if (gewicht == 1000) {
            open();
        }

    }
}
