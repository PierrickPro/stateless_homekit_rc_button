#include <Arduino.h>
#include <arduino_homekit_server.h>
#include "wifi_info.h"
#include <RCSwitch.h>

extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t cha_programmable_switch_event;

#define HOMEKIT_PROGRAMMABLE_SWITCH_EVENT_SINGLE_PRESS   0
#define HOMEKIT_PROGRAMMABLE_SWITCH_EVENT_LONG_PRESS     2

RCSwitch my_switch = RCSwitch();

unsigned long last_time = 0;
static uint32_t next_heap_millis = 0;

void rc_button_loop() {
  if (my_switch.available()) {
    unsigned long current_time = millis();
    unsigned long time_difference = current_time - last_time;
    uint8_t cha_value = 0;

    if (time_difference < 500) {
      Serial.println("Long click");
      Serial.println(time_difference);
      cha_value = HOMEKIT_PROGRAMMABLE_SWITCH_EVENT_LONG_PRESS;
    } else {
      Serial.println("Single click");
      Serial.println(time_difference);
      cha_value = HOMEKIT_PROGRAMMABLE_SWITCH_EVENT_SINGLE_PRESS;
    }
    
    last_time = current_time;
    
    my_switch.resetAvailable();

    cha_programmable_switch_event.value.uint8_value = cha_value;
    homekit_characteristic_notify(&cha_programmable_switch_event, cha_programmable_switch_event.value);
  }
}

void my_homekit_loop() {
    arduino_homekit_loop();
    const uint32_t t = millis();
    if (t > next_heap_millis) {
        // Show heap info every 5 seconds
        next_heap_millis = t + 5 * 1000;
        Serial.printf("Free heap: %d, HomeKit clients: %d\n", ESP.getFreeHeap(), arduino_homekit_connected_clients_count());
    }
}

// Called when the value is read by iOS Home APP
homekit_value_t cha_programmable_switch_event_getter() {
    return HOMEKIT_NULL_CPP();
}

void my_homekit_setup() {
    cha_programmable_switch_event.getter = cha_programmable_switch_event_getter;
    arduino_homekit_setup(&config);
}

void setup() {
    Serial.begin(115200);
    wifi_connect();
    //homekit_storage_reset();
    my_homekit_setup();
    my_switch.enableReceive(2);
}

void loop() {
  my_homekit_loop();
  rc_button_loop();
}