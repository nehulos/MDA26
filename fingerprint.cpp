#include <Adafruit_Fingerprint.h>
#include <HardwareSerial.h>
#include "config.h"

HardwareSerial fpSerial1(1);
HardwareSerial fpSerial2(2);

Adafruit_Fingerprint finger1(&fpSerial1);
Adafruit_Fingerprint finger2(&fpSerial2);

static constexpr int MAX_FINGERPRINT_ID = 162;

static bool fingerprintExists(Adafruit_Fingerprint& finger, int id)
{
    return finger.loadModel(id) == FINGERPRINT_OK;
}

static bool isFingerPresent(Adafruit_Fingerprint& finger)
{
    return finger.getImage() == FINGERPRINT_OK;
}

static int findFreeId()
{
    for (int id = 1; id <= MAX_FINGERPRINT_ID; id++)
    {
        bool used1 = fingerprintExists(finger1, id);
        bool used2 = fingerprintExists(finger2, id);

        if (!used1 && !used2)
            return id;
    }

    return -1;
}

bool sync_fingerprints()
{
    bool success = true;

    for (int id = 1; id <= MAX_FINGERPRINT_ID; id++)
    {
        bool exists1 = fingerprintExists(finger1, id);
        bool exists2 = fingerprintExists(finger2, id);

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

    fpSerial2.begin(
        57600,
        SERIAL_8N1,
        F2RXD_PIN,
        F2TXD_PIN
    );

    if (!finger1.verifyPassword())
        return false;

    if (!finger2.verifyPassword())
        return false;

    sync_fingerprints();

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
            int id = scan(finger1);

            if (id >= 0)
                return id;

            return scan(finger2);
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
        if (fingerprintExists(finger1, id))
            ok = false;
    }

    if (finger2.deleteModel(id) != FINGERPRINT_OK)
    {
        if (fingerprintExists(finger2, id))
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
    if (id < 1 || id > MAX_FINGERPRINT_ID)
        return false;

    auto finalize = [&](Adafruit_Fingerprint& finger) -> bool
    {
        if (finger.createModel() != FINGERPRINT_OK)
            return false;

        if (finger.storeModel(id) != FINGERPRINT_OK)
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

bool is_finger_present(int sensor_num)
{
    switch (sensor_num)
    {
        case 1:
            return isFingerPresent(finger1);

        case 2:
            return isFingerPresent(finger2);

        case 0:
            return isFingerPresent(finger1)
                || isFingerPresent(finger2);

        default:
            return false;
    }
}