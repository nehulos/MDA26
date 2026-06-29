#ifndef CONFIG_H
#define CONFIG_H

// ======================================================
// CONFIGURACIÓN DE HARDWARE
// ======================================================

// ------------------------------------------------------
// Comunicación con la PC
// ------------------------------------------------------

// UART0 (Serial)
constexpr int SERIAL_RX_PIN = 3;
constexpr int SERIAL_TX_PIN = 1;

// ------------------------------------------------------
// Sensores de huella
// ------------------------------------------------------

// UART1 (Serial1)
constexpr int F1RXD_PIN = 16; //8 amarillo
constexpr int F1TXD_PIN = 17; //9 negro

// UART2 (Serial2)
constexpr int F2RXD_PIN = 25;
constexpr int F2TXD_PIN = 26;

// ------------------------------------------------------
// LEDs de estado
// ------------------------------------------------------

constexpr int RED_PIN = 2;
constexpr int YELLOW_PIN = 4;
constexpr int GREEN_PIN = 5;

// ------------------------------------------------------
// Lector RFID MFRC522 (SPI)
// ------------------------------------------------------

constexpr int SS_PIN   = 21; // cambiado para evitar conflicto
constexpr int RST_PIN  = 22;

constexpr int SCLK_PIN = 18;
constexpr int MISO_PIN = 19;
constexpr int MOSI_PIN = 23;

// ------------------------------------------------------
// Lengüeta magnética (reed switch)
// ------------------------------------------------------

constexpr int REED_PIN = 27;

// ------------------------------------------------------
// Buzzer
// ------------------------------------------------------

constexpr int BZR_PIN = 32;

// ------------------------------------------------------
// Electroimán
// ------------------------------------------------------

/*RFID-RC522 (sesnor RFID)
nZA620_m5 (sensor de huella)
1N5408 (diodo)
lengueta magnética*/

constexpr int EMAG_PIN = 33;
constexpr int ENROLLMENT_TIMEOUT_MS = 10000;
constexpr int STATUS_INTERVAL_MS = 5000;

// ======================================================
// CONFIGURACIÓN DEL SISTEMA
// ======================================================

constexpr int MAX_FINGERPRINT_ID = 162;

constexpr char AUTH_NFC_FILE[] = "/authorized_nfc.txt";

constexpr unsigned long NFC_TIMEOUT = 100;

// ======================================================
// CONFIGURACIÓN WIFI
// ======================================================

constexpr char WIFI_SSID[]     = "ORIOnet_Test";
constexpr char WIFI_PASSWORD[] = "10293847";

// ======================================================
// CONFIGURACIÓN MQTT
// ======================================================

constexpr char MQTT_SERVER[] = "192.168.40.101";
constexpr int  MQTT_PORT     = 1883;

constexpr char MQTT_USER[] = "esp32";
constexpr char MQTT_PASS[] = "tomato";

#endif // CONFIG_H