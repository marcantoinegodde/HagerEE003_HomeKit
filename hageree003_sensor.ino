/*
 * hageree003_sensor.ino
 */

#include <Arduino.h>
#include <arduino_homekit_server.h>
#include "wifi_info.h"
#include <math.h>

#define LOG_D(fmt, ...)   printf_P(PSTR(fmt "\n") , ##__VA_ARGS__);

#define VIN 3300
#define VMCU 3030
#define R1 9920

void setup() {
    Serial.begin(115200);
    wifi_connect(); // in wifi_info.h
    //homekit_storage_reset();
    my_homekit_setup();
}

void loop() {
    my_homekit_loop();
    delay(10);
}

//==============================
// Homekit setup and loop
//==============================

// access your homekit characteristics defined in my_accessory.c
extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t cha_current_ambient_light_level;

static uint32_t next_heap_millis = 0;
static uint32_t next_report_millis = 0;

void my_homekit_setup() {
    arduino_homekit_setup(&config);
}

void my_homekit_loop() {
    arduino_homekit_loop();
    const uint32_t t = millis();
    if (t > next_report_millis) {
        // report sensor values every 10 seconds
        next_report_millis = t + 10 * 1000;
        my_homekit_report();
    }
    if (t > next_heap_millis) {
        // show heap info every 5 seconds
        next_heap_millis = t + 5 * 1000;
        LOG_D("Free heap: %d, HomeKit clients: %d", ESP.getFreeHeap(), arduino_homekit_connected_clients_count());
        
    }
}

void my_homekit_report() {
    float read = analogRead(A0);
    float Vout = (read * VMCU)/1024;
    float Rlux = (Vout*R1)/(VIN-Vout);
    float lux_value = pow(10, -1.17*log10(Rlux)+6.12);
    cha_current_ambient_light_level.value.float_value = lux_value;
    
    LOG_D("Current light level: %.1f", lux_value);
    homekit_characteristic_notify(&cha_current_ambient_light_level, cha_current_ambient_light_level.value);
}
