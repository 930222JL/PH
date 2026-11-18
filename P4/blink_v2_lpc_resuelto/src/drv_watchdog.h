#ifndef DRV_WATCHDOG_H
#define DRV_WATCHDOG_H

#include <stdint.h>

/* Arranca el watchdog con el timeout indicado (ms). Debe ser MAYOR que el
 * periodo de la alarma periodica del sistema, porque es lo que despierta
 * a la CPU del WFI y permite al lanzador alimentarlo. */
void drv_watchdog_iniciar(uint32_t timeout_ms);

/* Alimenta al watchdog. Llamar SOLO desde el bucle principal (lanzador),
 * nunca desde una RSI ni desde bucles secundarios. */
void drv_watchdog_alimentar(void);

/* Prueba: bloquea la ejecucion en un bucle infinito sin alimentar, para
 * comprobar que el watchdog resetea el sistema. */
void drv_watchdog_probar_bloqueo(void);

#endif
