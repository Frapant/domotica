#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "MQTT.h"
#include "hardware/gpio.h"

#include "scale.h"
#include "motor.h"

#define MQTT_TOPIC_PUB  "tap/snsr"

int PUB_INTERVAL_MS = 1000;

#define BTN 7

// actuator commando ontvangen
void Use_data(const char *topic, const char *payload){
    if (strcmp(topic, "tap/actr") == 0) {
        printf("Ontvangen payload: %s\n", payload);

        if (strcmp(payload, "open") == 0) {
            PUB_INTERVAL_MS = 10;
            open();
        }
        else if (strcmp(payload, "dicht") == 0) {
            dicht();
            PUB_INTERVAL_MS = 1000;
        }
    }
}


int main()
{
    setFeedBackRecieve(Use_data);

    stdio_init_all();
    mqtt_init();
    weigh_init();

    gpio_init(BTN);
    gpio_set_dir(BTN, GPIO_IN);
    gpio_pull_up(BTN);
    
    bool btn = 0;

    int teller = 0;
    absolute_time_t volgende = make_timeout_time_ms(PUB_INTERVAL_MS);

    while (1)
    {
        SyncMqttData();

        // altijd gewicht checken
        weigh_loop();
     
        if (time_reached(volgende))
        {

            char payload[128];
            snprintf(payload, sizeof(payload),
                     "%.2f", gewicht);

            mqtt_send(payload, MQTT_TOPIC_PUB);

            volgende = make_timeout_time_ms(PUB_INTERVAL_MS);
        }
        sleep_ms(2);
    }
}