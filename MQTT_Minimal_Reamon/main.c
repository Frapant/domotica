#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "MQTT.h"

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

        if (time_reached(volgende))
        {
            char payload[128];
            snprintf(payload, sizeof(payload),
                     "{\"bron\":\"rp2350\",\"teller\":%d}", teller++);

            mqtt_send(payload, MQTT_TOPIC_PUB);

            volgende = make_timeout_time_ms(PUB_INTERVAL_MS);
        }

        sleep_ms(2);
    }
}