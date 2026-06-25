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

#include "electromagnet.h"
//void init_electromagnet()
//void lock_door()
//void unlock_door()
//void unlock_door_timed(int ms)

#include "WiFiMQTT.h"
//namespace wifi
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

void setup(){;

    initLeds({RED_PIN, YELLOW_PIN, GREEN_PIN});

    setLed(1,true)

    init_fingerprint()

    setLed(2,500,0,1)

    init_electromagnet()

    setLed(2,500,0,1)
  
}

void loop()
{

}