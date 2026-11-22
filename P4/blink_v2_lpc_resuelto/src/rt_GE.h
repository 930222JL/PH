#ifndef RT_GE_H 
#define RT_GE_H
#include "rt_evento_t.h"
#include <stdint.h>

#define rt_GE_MAX_SUSCRITOS 4

void rt_GE_iniciar(void);

void rt_GE_lanzador(void);

void rt_GE_actualizar(EVENTO_T ID_evento, uint32_t auxiliar);

#endif
