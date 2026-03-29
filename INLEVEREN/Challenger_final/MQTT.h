#define DEBUG true

#if DEBUG
  #define DEBUG_PRINT(fmt, ...) printf("[DEBUG] " fmt, ##__VA_ARGS__)
#else
  #define DEBUG_PRINT(fmt, ...) // Doet helemaal niets
#endif

#ifndef _MQTT_MAIN_
#define _MGTT_MAIN_

void SyncMqttData();

bool mqtt_init();

bool mqtt_send(char data[], char topic[]);

void setMQTToutput(bool Output);

typedef void (*mqtt_message_sent_function)(
    const char *topic,
    const char *payload);

typedef void (*mqtt_message_recieved_function)(
    const char *topic,
    const char *payload);

void setFeedBackSent(mqtt_message_sent_function function);

void setFeedBackRecieve(mqtt_message_recieved_function function);


#endif