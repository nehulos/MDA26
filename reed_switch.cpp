#include <Arduino.h>
#include "reed_switch.h"
#include "config.h"

/**
 * Configura el GPIO 12 (RXD_PIN) como entrada.
 */
void init_reed() {
    // Usamos INPUT_PULLUP si el sensor conecta a GND al cerrarse
    pinMode(REED_PIN, INPUT_PULLUP); 
}

/**
 * Cambia el multiplexor al canal de la lengüeta y lee su estado.
 */
bool is_door_open() {
    
    // Pequeña pausa para estabilización de la señal en el multiplexor
    delayMicroseconds(50);

    // 2. Leer el estado del pin GPIO 12
    // Normalmente, un Reed Switch devuelve HIGH cuando el imán se aleja (puerta abierta)
    if (digitalRead(REED_PIN) == HIGH) {
        return true;
    }
    return false;
}
