#pragma once

#include <WiFi.h>
#include <PubSubClient.h>

namespace wifi
{
    using MessageCallback =
        void (*)(char* topic, byte* payload, unsigned int length);

    void begin(MessageCallback callback);
    void update();

    bool connected();

    bool publish(
        const char* topic,
        const char* payload,
        bool retained = false
    );

    bool subscribe(const char* topic);

    IPAddress localIP();

    PubSubClient& mqtt();
}