#ifndef REED_SWITCH_H
#define REED_SWITCH_H

/**
 * Inicializa el pin compartido para la lectura del sensor magnético.
 */
void init_reed();

/**
 * Verifica si la puerta se encuentra abierta.
 * @return true si está abierta, false si está cerrada.
 */
bool is_door_open();

#endif