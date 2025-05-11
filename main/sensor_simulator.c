#include <stdlib.h>

float get_fake_temperature(void) {
    return 20.0 + (rand() % 1000) / 100.0f;  // 20.0 - 30.0
}

float get_fake_humidity(void) {
    return 40.0 + (rand() % 2000) / 100.0f;  // 40.0 - 60.0
}

float get_fake_pressure(void) {
    return 1000.0 + (rand() % 500) / 10.0f;  // 1000 - 1050
}
