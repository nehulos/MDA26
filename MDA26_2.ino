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
}

void setup(){;

    //load communication with PC
    Serial.begin(115200);
    Serial.setRxBufferSize(1024); 
    Serial.println("hello world");

    //load modules
    initLeds({RED_PIN, YELLOW_PIN, GREEN_PIN});
    setLed(3,true);
    Serial.print("leds loaded");

    init_fingerprint();
    blinkLed(2,500,0,1);
    Serial.print("fingerprints loaded");

    init_electromagnet()
    blinkLed(2,500,0,1)
    Serial.print("electromagnet loaded");

    init_reed()
    blinkLed(2,500,0,1)
    Serial.print("reed loaded");

    Serial.println();
    network::begin(onMessage)
    blinkLed(2,500,0,1)
    Serial.print("network loaded");
    Serial.print("ESP32 IP: ");
    Serial.println(network::localIP());
    Serial.println();

    //subscribe to topics
    network::subscribe("mda26/door");
    network::subscribe("mda26/fingerprint");
    network::subscribe("mda26/door");

    //sync fingerprints
    sync_fingerprints();

    //send fingerprints to server
    for_each_fingerprint_id(0, [](int id){
        char payload[10];
        sprintf(payload, "%d", id);
        network::publish("mda26/fingerprint/exists", payload, true);
    });

    //state machine
    int state = 0
    //0 = waiting
    //10 = enrollment first step
    //11 = enrollment second step

    //timers
    uint32_t statusTime = millis();
    uint32_t enrollmentTime = millis();
    uint32_t doorTime = -1; //-1 means no door timer is active

    setLed(3,false);

}

void unlock_door_timed(int ms){
    setLed(3,true);
    unlock_door();
    doorTime = millis() + ms;
}

void begin_enrollment(){
    state = 10;
    enrollmentTime = millis();
    blinkLed(2, 100, 100, 3);
    setLed(2, true);
}

void end_enrollment(){
    state = 0;
    blinkLed(1, 100, 100, 3);
    setLed(2, false);
}

void loop(){

    if doorTime != -1 and millis() > doorTime {
        lock_door();
        doorTime = -1;
        setLed(3,false);
    }

    if state == 0 {
        //waiting for events
        if is_finger_present(0) {
            int id = get_fingerprint_id(0);
            if id >= 0 {
                network::publish("mda26/fingerprint/scan", String(id).c_str());
                unlock_door_timed(5000);
            } else {
                blinkLed(1, 100, 100, 2);
            }
        }
    } else if state == 10 {
        //enrollment first step
        if is_finger_present(0) {
            capture_fingerprint(0, 1);
            state = 11;
            enrollmentTime = millis();
            blinkLed(3, 100, 100, 3);
        } else if millis() - enrollmentTime > ENROLLMENT_TIMEOUT_MS {
            end_enrollment();
        }
    } else if state == 11 {
        //enrollment second step
        if is_finger_present(0) {
            capture_fingerprint(0, 2);
            int newId = get_next_available_id();
            finalize_enrollment(0, newId);
            network::publish("mda26/fingerprint/enrolled", String(newId).c_str());
            state = 0;
            blinkLed(3, 100, 100, 5);
            setLed(2, false);
        } else if millis() - enrollmentTime > ENROLLMENT_TIMEOUT_MS {
            end_enrollment();
        }
    }

    if (millis() - statusTime > STATUS_INTERVAL_MS) {
        statusTime = millis();
        network::publish("mda26/status", is_door_open() ? "open" : "closed");
    }

}