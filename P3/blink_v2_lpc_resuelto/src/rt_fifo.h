#ifndef RT_FIFO_H
#define RT_FIFO_H
#include "rt_evento_t.h"
#include "drv_tiempo.h"
#include <stdint.h>

#define RT_FIFO_TAM 32u

void rt_FIFO_inicializar(uint32_t monitor_overflow);

void rt_FIFO_encolar(uint32_t ID_evento, uint32_t auxData);

uint8_t rt_FIFO_extraer(EVENTO_T *ID_evento,uint32_t *auxData,Tiempo_us_t *TS);

uint32_t rt_FIFO_estadisticas(EVENTO_T ID_evento);

#endif