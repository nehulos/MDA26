#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

namespace network
{
    using MessageCallback = void (*)(char* topic, byte* payload, unsigned int length);

    void begin(MessageCallback callback);

    void update();

    bool connected();

    bool publish(
        const char* topic,
        const char* payload,
        bool retained = false
    );

    bool publish(
        const char* topic,
        int payload,
        bool retained = false
    );

    bool subscribe(
        const char* topic
    );

    IPAddress localIP();

    PubSubClient& mqtt();
}