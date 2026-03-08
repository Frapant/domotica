#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "MQTT.h"

#include "scale.h"
#include "motor.h"

#define MQTT_TOPIC_PUB  "Makerspace/Logger01"
#define PUB_INTERVAL_MS 5000


int main()
{
    stdio_init_all();
    mqtt_init();

    int teller = 0;
    absolute_time_t volgende = make_timeout_time_ms(PUB_INTERVAL_MS);

    while (1)
    {
        SyncMqttData();

         // altijd gewicht checken
        // weigh_loop();

        // voor monitor testing
        // printf("Gewogen gewicht: %.2f\n", gewicht);

        // kraan open bij gewicht en na 5 seconden dicht
        // aanpassen naar mqtt_topic received iets ofzo
        
        if (gewicht == 1000) {
            open();
            sleep_ms(5000);
            dicht();
        }

        if (time_reached(volgende))
        {
            weigh_loop();

            char payload[128];
            snprintf(payload, sizeof(payload),
                     "{\"bron\":\"rp2350\",\"teller\":%d}", teller++);

            mqtt_send(payload, MQTT_TOPIC_PUB);

            volgende = make_timeout_time_ms(PUB_INTERVAL_MS);
        }

        sleep_ms(2);
    }
}