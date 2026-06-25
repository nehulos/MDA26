#ifndef ELECTROMAGNET_H
#define ELECTROMAGNET_H

#include <Arduino.h>
#include "config.h"

/**
 * Inicializa el pin de control de la cerradura y establece el estado inicial.
 */
void init_electromagnet();

/**
 * Activa el paso de corriente al electroimán para bloquear la puerta.
 */
void lock_door();

/**
 * Corta la alimentación del electroimán para liberar la placa metálica.
 */
void unlock_door();

/**
 * Realiza una apertura temporal (ideal para accesos concedidos).
 * @param ms Tiempo en milisegundos que permanecerá abierta.
 */
void unlock_door_timed(int ms);

#endif
