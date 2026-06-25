#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>
#include "config.h"

/**
 * Inicializa el pin del buzzer como salida.
 */
void init_buzzer();

/**
 * Genera un pitido corto y agudo para indicar éxito (ej. acceso concedido).
 */
void beep_ok();

/**
 * Genera una secuencia de pitidos para indicar un error (ej. huella no reconocida).
 */
void beep_error();

/**
 * Activa el buzzer de forma continua.
 */
void buzzer_on();

/**
 * Desactiva el buzzer.
 */
void buzzer_off();

#endif