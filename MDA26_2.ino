//INCLUIMOS MÓDULOS
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
    blinkLed(3,true);
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
  
}

void loop(){

}