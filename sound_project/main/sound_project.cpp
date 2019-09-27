#include "Arduino.h"

void setup() {
    Serial.begin(115200);
}

void loop() {
    printf("loop\n");
    delay(1000);
}