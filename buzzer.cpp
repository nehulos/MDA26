#include "buzzer.h"

/**
 * Configura el GPIO 16 (BZR_PIN) como salida.
 */
void init_buzzer() {
    pinMode(BZR_PIN, OUTPUT);
    digitalWrite(BZR_PIN, LOW); // Asegurar que inicie apagado
}

void buzzer_on() {
    digitalWrite(BZR_PIN, HIGH);
}

void buzzer_off() {
    digitalWrite(BZR_PIN, LOW);
}

/**
 * Sonido de éxito: Un pitido rápido de 100ms.
 */
void beep_ok() {
    buzzer_on();
    delay(100);
    buzzer_off();
}

/**
 * Sonido de error: Dos pitidos largos de 300ms.
 */
void beep_error() {
    for (int i = 0; i < 2; i++) {
        buzzer_on();
        delay(300);
        buzzer_off();
        if (i == 0) delay(100); // Pausa entre pitidos
    }
}