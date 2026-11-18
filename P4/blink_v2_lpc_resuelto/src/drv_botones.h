#ifndef DRV_BOTONES_H
#define DRV_BOTONES_H
#include "rt_evento_t.h"
#include <stdint.h>

void drv_botones_iniciar(void (*cb_a_llamar)(EVENTO_T ID_evento,uint32_t auxData));

void drv_botones_pulsar_boton(uint32_t pin,uint32_t auxData);

void drv_botones_actualizar(EVENTO_T ID_evento,uint32_t auxData);

#endif