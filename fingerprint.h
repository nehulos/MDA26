#pragma once

#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>

extern HardwareSerial fpSerial1;
extern HardwareSerial fpSerial2;

extern Adafruit_Fingerprint finger1;
extern Adafruit_Fingerprint finger2;

extern bool finger1Available;
extern bool finger2Available;

// Initialization / maintenance
bool init_fingerprint();
bool sync_fingerprints();
int get_next_available_id();

// Identification
int get_fingerprint_id(int sensor_num);
bool is_finger_present(int sensor_num);

// Enrollment
bool capture_fingerprint(int sensor_num, int buffer);
bool finalize_enrollment(int sensor_num, int id);

// Syncronization
typedef void (*FingerprintIdCallback)(int id);

void for_each_fingerprint_id(
    int sensorId,
    FingerprintIdCallback callback
);


// Database management
bool delete_fingerprint(int id);