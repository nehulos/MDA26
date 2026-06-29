#include "electromagnet.h"

/**
 * Configura el EMAG_PIN (GPIO 2) como salida [2].
 * El estado inicial por defecto es bloqueado para garantizar seguridad al arrancar.
 */
void init_electromagnet() {
    pinMode(EMAG_PIN, OUTPUT);
    lock_door(); 
}

/**
 * Bloquea la puerta activando el relé.
 * Basado en el principio de que la cerradura EL-280 requiere 12V continuos para retener [5, 8].
 */
void lock_door() {
    // Si el relé es Low-Level Trigger, LOW lo activa [7]
    digitalWrite(EMAG_PIN, LOW); 
}

/**
 * Desbloquea la puerta cortando la alimentación del electroimán.
 */
void unlock_door() {
    // HIGH desactiva el relé (corta el circuito de 12V)
    digitalWrite(EMAG_PIN, HIGH);
}