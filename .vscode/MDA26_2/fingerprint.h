#ifndef FINGERPRINT_H
#define FINGERPRINT_H

#include <Adafruit_Fingerprint.h>
#include "multiplexer.h"
#include "config.h"

// Function prototypes
bool init_fingerprint();
int get_fingerprint_id(int sensor_num);
int enroll_fingerprint(int sensor_num, int id);
bool delete_fingerprint(int id);

#endif