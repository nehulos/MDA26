#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>
#include "config.h"

bool finger1Available = false;
bool finger2Available = false;

HardwareSerial fpSerial1(1);
HardwareSerial fpSerial2(2);

Adafruit_Fingerprint finger1 = Adafruit_Fingerprint(&fpSerial1);
Adafruit_Fingerprint finger2 = Adafruit_Fingerprint(&fpSerial2);


static bool fingerprintExists(int sensorNum, int id)
{
    switch (sensorNum)
    {
        case 1:
            return finger1Available
                && finger1.loadModel(id) == FINGERPRINT_OK;

        case 2:
            return finger2Available
                && finger2.loadModel(id) == FINGERPRINT_OK;

        default:
            return false;
    }
}

static bool isFingerPresent(Adafruit_Fingerprint& finger)
{
    return finger.getImage() == FINGERPRINT_OK;
}

int get_next_available_id()
{
    for (int id = 1; id <= MAX_FINGERPRINT_ID; id++)
    {
        bool used1 =
            finger1Available &&
            fingerprintExists(1, id);

        bool used2 =
            finger2Available &&
            fingerprintExists(2, id);

        if (!used1 && !used2)
            return id;
    }

    return -1;
}

bool sync_fingerprints()
{
    if (!finger1Available || !finger2Available)
        return true;
    
    bool success = true;

    for (int id = 1; id <= MAX_FINGERPRINT_ID; id++)
    {
        bool exists1 = fingerprintExists(1, id);
        bool exists2 = fingerprintExists(2, id);

        if (exists1 != exists2)
        {
            finger1.deleteModel(id);
            finger2.deleteModel(id);

            success = false;
        }
    }

    return success;
}

bool init_fingerprint()
{
    fpSerial1.begin(
        57600,
        SERIAL_8N1,
        F1RXD_PIN,
        F1TXD_PIN
    );
    Serial.printf("UART1 RX=%d TX=%d\n", F1RXD_PIN, F1TXD_PIN);

    fpSerial2.begin(
        57600,
        SERIAL_8N1,
        F2RXD_PIN,
        F2TXD_PIN
    );

    finger1.begin(57600);
    finger2.begin(57600);

    delay(100);

    finger1Available = finger1.verifyPassword();
    finger2Available = finger2.verifyPassword();

    uint8_t p = finger1.getParameters();

    Serial.printf("Finger1 getParameters: 0x%02X\n", p);

    if (finger1Available)
        Serial.println("Fingerprint sensor 1 detected");
    else
        Serial.println("Fingerprint sensor 1 not detected");

    if (finger2Available)
        Serial.println("Fingerprint sensor 2 detected");
    else
        Serial.println("Fingerprint sensor 2 not detected");

    if (!finger1Available && !finger2Available)
    {
        Serial.println("ERROR: No fingerprint sensors detected");
        return false;
    }

    if (finger1Available != finger2Available)
    {
        Serial.println(
            "WARNING: Only one fingerprint sensor connected"
        );
    }

    if (finger1Available && finger2Available)
    {
        sync_fingerprints();
    }

    return true;
}

int get_fingerprint_id(int sensor_num)
{
    auto scan = [](Adafruit_Fingerprint& finger) -> int
    {
        if (finger.getImage() != FINGERPRINT_OK)
            return -1;

        if (finger.image2Tz(1) != FINGERPRINT_OK)
            return -1;

        if (finger.fingerFastSearch() != FINGERPRINT_OK)
            return -1;

        return finger.fingerID;
    };

    switch (sensor_num)
    {
        case 1:
            return scan(finger1);

        case 2:
            return scan(finger2);

        case 0:
{
            if (finger1Available)
            {
                int id = scan(finger1);

                if (id >= 0)
                    return id;
            }

            if (finger2Available)
            {
                int id = scan(finger2);

                if (id >= 0)
                    return id;
            }

            return -1;
        }

        default:
            return -1;
    }
}

bool delete_fingerprint(int id)
{
    bool ok = true;

    if (finger1.deleteModel(id) != FINGERPRINT_OK)
    {
        if (fingerprintExists(1, id))
            ok = false;
    }

    if (finger2.deleteModel(id) != FINGERPRINT_OK)
    {
        if (fingerprintExists(2, id))
            ok = false;
    }

    return ok;
}


bool capture_fingerprint(int sensor_num, int buffer)
{
    if (buffer != 1 && buffer != 2)
        return false;

    auto capture = [&](Adafruit_Fingerprint& finger) -> bool
    {
        if (finger.getImage() != FINGERPRINT_OK)
            return false;

        if (finger.image2Tz(buffer) != FINGERPRINT_OK)
            return false;

        return true;
    };

    switch (sensor_num)
    {
        case 1:
            return capture(finger1);

        case 2:
            return capture(finger2);

        default:
            return false;
    }
}

bool finalize_enrollment(int sensor_num, int id)
{
    Serial.print("trying to finish enrollment, id: "); 
    Serial.print(id);
    Serial.print(", sensor num is "); 
    Serial.println(sensor_num);

    if (id < 1 || id > MAX_FINGERPRINT_ID)
        Serial.print("indicated id is out of bound: ");
        Serial.println(id);
        return false;

    auto finalize = [&](Adafruit_Fingerprint& finger) -> bool
    {
        if (finger.createModel() != FINGERPRINT_OK)
            Serial.print("could not create model");
            return false;

        if (finger.storeModel(id) != FINGERPRINT_OK)
            Serial.print("could not store model");
            return false;

        return true;
    };

    switch (sensor_num)
    {
        case 1:
            return finalize(finger1);

        case 2:
            return finalize(finger2);

        default:
            return false;
    }
}

int is_finger_present(int sensor_num)
{
    switch (sensor_num)
    {
        case 1:
            if(finger1Available && isFingerPresent(finger1)){
                return 1;
            }

        case 2:
            if(finger2Available && isFingerPresent(finger2)){
                return 2;
            }
        case 0:
            return (finger1Available && isFingerPresent(finger1)) ||
                   (finger2Available && isFingerPresent(finger2));

        default:
            return -1;
    }
    return -1;
}

typedef void (*FingerprintIdCallback)(int id);

void for_each_fingerprint_id(
    int sensorId,
    FingerprintIdCallback foundCallback,
    FingerprintIdCallback missingCallback
)
{
    Adafruit_Fingerprint& finger =
    (sensorId == 2 && finger2Available) ? finger2 : finger1;

    for (int fingerprintId = 1; fingerprintId <= MAX_FINGERPRINT_ID; fingerprintId++)
    {
        if (finger.loadModel(fingerprintId) == FINGERPRINT_OK)
        {
            foundCallback(fingerprintId);
        }else{
            missingCallback(fingerprintId);
        }
    }
}
