//include libs
#include <time.h>

//include modules
#include "config.h"

#include "leds.h"
//void initLeds([int])
//void setLed(index, valor:bool)
//void blinkLed(index, tiempo encendido, tiempo apagado, repeticiones)

#include "fingerprint.h"
//bool init_fingerprint();
//bool sync_fingerprints();
//int get_fingerprint_id(int sensor_num);
//bool is_finger_present(int sensor_num);
//bool capture_fingerprint(int sensor_num, int buffer);
//bool finalize_enrollment(int sensor_num, int id);
//bool delete_fingerprint(int id);
//void for_each_fingerprint_id(int sensorId, callback) -> (int id)

#include "electromagnet.h"
//void init_electromagnet()
//void lock_door()
//void unlock_door()
//void unlock_door_timed(int ms)

#include "WiFiMQTT.h"
//namespace network
//void begin(MessageCallback callback);
//void update();
//bool connected();
//bool publish(const char* topic, const char* payload, bool retained = false);
//bool subscribe(const char* topic);
//IPAddress localIP();
//PubSubClient& mqtt();

//#include "nfc.h"

#include "reed_switch.h"
//void init_reed()
//bool is_door_open()

///////////////////////////////////////////////////////////

enum State
{
    STATE_IDLE,
    STATE_ENROLL_FIRST,
    STATE_ENROLL_SECOND
};

State state = STATE_IDLE;

// timers
uint32_t statusTime = 0;
uint32_t enrollmentTime = 0;
uint32_t doorUnlockUntil = 0;
uint32_t fingerCheckTime  = 0;

bool doorTimerActive = false;
bool fingerWasPresent = false;
int lastPresentFinger = -1;

int lastEnrolledFinger = -1;
int lastEnrolledId = -1;
//due to limitations, user must enroll the same finger on both sensors
//user must place the finger four times to complete enrollment, two times on each sensor
//STATE_ENROLL_FIRST, lastEnrolledFinger = -1
//STATE_ENROLL_SECOND, lastEnrolledFinger = -1
//STATE_ENROLL_FIRST, lastEnrolledFinger > 0
//STATE_ENROLL_SECOND, lastEnrolledFinger > 0

///////////////////////////////////////////////////////////

void unlock_door_timed(int ms)
{
    setLed(2, true);

    unlock_door();

    doorUnlockUntil = millis() + ms;
    doorTimerActive = true;
}

void begin_enrollment()
{
    state = STATE_ENROLL_FIRST;

    enrollmentTime = millis();

    blinkLed(1, 100, 100, 3);
    setLed(1, true);
}

void end_enrollment()
{
    state = STATE_IDLE;
    lastEnrolledFinger = -1;
    lastEnrolledId = -1;

    blinkLed(0, 100, 100, 3);

    setLed(1, false);
}

///////////////////////////////////////////////////////////

static bool payload_equals(
    const byte* payload,
    unsigned int length,
    const char* text
)
{
    return strlen(text) == length &&
           memcmp(payload, text, length) == 0;
}

static int payload_to_int(
    const byte* payload,
    unsigned int length
)
{
    char buffer[16];

    size_t copyLen = min(
        (size_t)length,
        sizeof(buffer) - 1
    );

    memcpy(buffer, payload, copyLen);

    buffer[copyLen] = '\0';

    return atoi(buffer);
}

///////////////////////////////////////////////////////////

void onMessage(
    char* topic,
    byte* payload,
    unsigned int length
)
{
    Serial.print("Message received on ");
    Serial.println(topic);

    Serial.print("Payload: ");

    for (unsigned int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }

    Serial.println();

    ///////////////////////////////////////////////////////

    if (strcmp(topic, "mda26/door") == 0)
    {
        //if payload is "open", unlock the door
        if (payload_equals(payload, length, "open"))
        {
            unlock_door();
        }

        //if payload is "close", lock the door
        else if (payload_equals(payload, length, "close"))
        {
            lock_door();
        }

        //if payload is a number, unlock the door for that many milliseconds
        else if (length > 0)
        {
            int ms = payload_to_int(payload, length);

            if (ms > 0)
            {
                unlock_door_timed(ms);
            }
        }
    }

    ///////////////////////////////////////////////////////

    else if (strcmp(topic, "mda26/fingerprint") == 0)
    {
        if (payload_equals(payload, length, "enroll"))
        {
            begin_enrollment();
        }
    }

    ///////////////////////////////////////////////////////

    else if (strcmp(topic, "mda26/fingerprint/remove") == 0)
    {
        if (length > 0)
        {
            int id = payload_to_int(payload, length);

            if (id >= 0)
            {
                bool success = delete_fingerprint(id);

                char buffer[12];
                sprintf(buffer, "%d", id);

                if(success){
                    network::publish(
                        "mda26/fingerprint/removed",
                        buffer
                    );
                }else{
                    network::publish(
                        "mda26/fingerprint/remove_failed",
                        buffer
                    );
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////

void setup()
{
    //load communication with PC
    Serial.begin(115200);
    Serial.setRxBufferSize(1024);

    Serial.println("hello world");

    ///////////////////////////////////////////////////////

    //load modules

    initLeds({
        RED_PIN,
        YELLOW_PIN,
        GREEN_PIN
    });

    setLed(2, true);

    Serial.println("leds loaded");

    ///////////////////////////////////////////////////////

    init_fingerprint();

    blinkLed(1, 500, 0, 1);

    Serial.println("fingerprints loaded");

    ///////////////////////////////////////////////////////

    init_electromagnet();

    blinkLed(1, 500, 0, 1);

    Serial.println("electromagnet loaded");

    ///////////////////////////////////////////////////////

    init_reed();

    blinkLed(1, 500, 0, 1);

    Serial.println("reed loaded");

    ///////////////////////////////////////////////////////

    network::begin(onMessage);

    blinkLed(1, 500, 0, 1);

    Serial.println("network loaded");

    Serial.print("ESP32 IP: ");
    Serial.println(network::localIP());

    Serial.println();

    ///////////////////////////////////////////////////////

    //subscribe to topics

    network::subscribe("mda26/door");

    network::subscribe("mda26/fingerprint");

    network::subscribe("mda26/fingerprint/remove");

    ///////////////////////////////////////////////////////

    //send fingerprints to server

    for_each_fingerprint_id(
        0,
        [](int id)
        {
            char payload[12];

            sprintf(payload, "%d", id);

            network::publish(
                "mda26/fingerprint/exists",
                payload,
                false
            );
        }
    );
    network::publish(
        "mda26/fingerprint/exists",
        -1,
        false
    );

    ///////////////////////////////////////////////////////

    statusTime = millis();

    setLed(2, false);
}


///////////////////////////////////////////////////////////

//void updateDoor();
//void updateEnrollment();
//void updateNetwork();


void loop()
{
    delay(50);

    network::update();

    if (
        millis() - statusTime >
        STATUS_INTERVAL_MS
    )
    {
        statusTime = millis();

        network::publish(
            "mda26/status",
            is_door_open()
                ? "open"
                : "closed"
        );
    }

    ///////////////////////////////////////////////////////

    if (
        doorTimerActive &&
        (int32_t)(millis() - doorUnlockUntil) >= 0
    )
    {
        lock_door();

        doorTimerActive = false;

        setLed(2, false);
    }

    ///////////////////////////////////////////////////////

    int fingerPresent = lastPresentFinger;

    if (
        fingerCheckTime == 0 ||
        millis() - fingerCheckTime > 500
    )
    {
        fingerCheckTime = millis();
        fingerPresent = is_finger_present(0);
    }

    bool isFingerPresent = fingerPresent > 0;

    if(isFingerPresent && fingerPresent == lastEnrolledFinger){
        //finger is present and is the last enrolled finger
        //do not allow to enroll again
        blinkLed(
            0,
            100,
            100,
            5
        );
        //end_enrollment();
        if (
            millis() - enrollmentTime >
            ENROLLMENT_TIMEOUT_MS
        )
        {
            end_enrollment();
        }
        //freeze state until finger is removed or timeout occurs
        fingerWasPresent =
            isFingerPresent;

        lastPresentFinger  =
            fingerPresent;
        return;
    }

    ///////////////////////////////////////////////////////

    if (state == STATE_IDLE)
    {
        //waiting for events

        if (isFingerPresent && !fingerWasPresent)
        {
            int id =
                get_fingerprint_id(0);

            if (id >= 0)
            {
                char payload[12];

                sprintf(payload, "%d", id);

                network::publish(
                    "mda26/fingerprint/scan",
                    payload
                );

                unlock_door_timed(5000);
            }
            else
            {
                blinkLed(
                    0,
                    100,
                    100,
                    2
                );
            }
        }
    }

    ///////////////////////////////////////////////////////

    else if (state == STATE_ENROLL_FIRST)
    {
        //enrollment first step

        if (isFingerPresent)
        {
            if (
                capture_fingerprint(
                    fingerPresent,
                    1
                )
            )
            {
                state =
                    STATE_ENROLL_SECOND;

                enrollmentTime =
                    millis();

                blinkLed(
                    2,
                    100,
                    100,
                    3
                );
            }
        }
        else if (
            millis() - enrollmentTime >
            ENROLLMENT_TIMEOUT_MS
        )
        {
            end_enrollment();
        }
    }

    ///////////////////////////////////////////////////////

    else if (state == STATE_ENROLL_SECOND)
    {
        //enrollment second step

        if (isFingerPresent)
        {
            if (
                capture_fingerprint(
                    fingerPresent,
                    2
                )
            )
            {
                int newId =
                    get_next_available_id();

                if (lastEnrolledId >= 0){
                    //the last enrolled id is still available
                    //use it again
                    newId = lastEnrolledId;
                }

                int oldEnrolledId = lastEnrolledId;
                lastEnrolledId = newId;

                if (
                    finalize_enrollment(
                        fingerPresent,
                        newId
                    )
                )
                {
                    char payload[12];

                    sprintf(
                        payload,
                        "%d",
                        newId
                    );

                    network::publish(
                        "mda26/fingerprint/enrolled",
                        payload
                    );

                    blinkLed(
                        2,
                        100,
                        100,
                        5
                    );
                }
                else
                {
                    blinkLed(
                        0,
                        100,
                        100,
                        5
                    );

                    //retry enrollment step if finalize_enrollment fails
                    fingerWasPresent =
                        isFingerPresent;

                    lastPresentFinger  =
                        fingerPresent;
                    return;
                }

                //check if next fingerprint is available, if not, end enrollment
                bool available =
                (
                    fingerPresent == 2 &&
                    finger1Available
                )
                ||
                (
                    fingerPresent == 1 &&
                    finger2Available
                );
                
                if (oldEnrolledId >= 0 || !available){ 
                    //finger pressed four times if sensor available
                    //finger pressed two times if sensor unavailable
                    setLed(1, false);
                    state = STATE_IDLE;
                    lastEnrolledFinger = -1;
                    lastEnrolledId = -1;
                }else{
                    //finger pressed two times
                    //after first enrollment, wait for second enrollment
                    lastEnrolledFinger =
                        fingerPresent;

                    state = STATE_ENROLL_FIRST;
                }
            
            }
        }
        else if (
            millis() - enrollmentTime >
            ENROLLMENT_TIMEOUT_MS
        )
        {
            end_enrollment();
        }
    }

    ///////////////////////////////////////////////////////

    fingerWasPresent =
        isFingerPresent;

    lastPresentFinger  =
        fingerPresent;

    ///////////////////////////////////////////////////////
}