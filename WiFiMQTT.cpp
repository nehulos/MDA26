#include "WiFiMQTT.h"

#include <WiFi.h>
#include <PubSubClient.h>

#include "config.h"

namespace network
{
    static WiFiClient wifiClient;
    static PubSubClient mqttClient(wifiClient);

    static MessageCallback callbackFunction = nullptr;

    static void connectWiFi()
    {
        if (WiFi.status() == WL_CONNECTED)
            return;

        Serial.println("Connecting WiFi...");

        WiFi.mode(WIFI_STA);
        WiFi.begin(
            WIFI_SSID,
            WIFI_PASSWORD
        );

        while (WiFi.status() != WL_CONNECTED)
        {
            delay(500);
        }

        Serial.print("WiFi connected. IP: ");
        Serial.println(WiFi.localIP());
    }

    static void connectMQTT()
    {
        while (!mqttClient.connected())
        {
            Serial.println("Connecting MQTT...");

            if (mqttClient.connect(
                    "ESP32",
                    MQTT_USER,
                    MQTT_PASS))
            {
                Serial.println("MQTT connected");

                //mqttClient.subscribe(
                //    TOPIC_CMD
                //);

                publish(
                    "mda26/status",
                    "connected",
                    true
                );
            }
            else
            {
                Serial.print("MQTT failed, rc=");
                Serial.println(mqttClient.state());
                delay(1000);
            }
        }
    }

    void begin(MessageCallback callback)
    {
        callbackFunction = callback;

        mqttClient.setServer(
            MQTT_SERVER,
            MQTT_PORT
        );

        mqttClient.setCallback(
        [](char* topic, byte* payload, unsigned int length)
        {
            Serial.println("MQTT CALLBACK");

            if (callbackFunction)
                callbackFunction(topic, payload, length);
        });

        connectWiFi();
        connectMQTT();
    }

    void update()
    {
        if (WiFi.status() != WL_CONNECTED)
            connectWiFi();

        if (!mqttClient.connected())
            connectMQTT();

        mqttClient.loop();
    }

    bool connected()
    {
        return
            WiFi.status() == WL_CONNECTED &&
            mqttClient.connected();
    }

    bool publish(
            const char* topic,
            const char* payload,
            bool retained
        )
    {
        Serial.print("publishing ");
        Serial.print(topic);
        Serial.print(", ");
        Serial.print(payload);
        Serial.print(", ");
        Serial.println(retained ? "true" : "false");
        return mqttClient.publish(topic, payload, retained);
    }

    bool publish(
            const char* topic,
            int payload,
            bool retained
        )
    {
        char buffer[12];
        snprintf(buffer, sizeof(buffer), "%d", payload);

        Serial.print("publishing ");
        Serial.print(topic);
        Serial.print(", ");
        Serial.print(payload);
        Serial.print(", ");
        Serial.println(retained ? "true" : "false");
        return mqttClient.publish(topic, buffer, retained);
    }

    bool subscribe(const char* topic)
    {
        bool ok = mqttClient.subscribe(topic);

        Serial.print("subscribe ");
        Serial.print(topic);
        Serial.print(" -> ");
        Serial.println(ok ? "OK" : "FAILED");

        return ok;
    }

    IPAddress localIP()
    {
        return WiFi.localIP();
    }

    PubSubClient& mqtt()
    {
        return mqttClient;
    }
}
