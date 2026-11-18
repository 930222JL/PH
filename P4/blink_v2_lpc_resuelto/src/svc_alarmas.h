#ifndef SVC_ALARMAS_H
#define SVC_ALARMAS_H
#include "rt_evento_t.h"
#include <stdint.h>
#include <stdbool.h>
#include "drv_tiempo.h"

#define svc_ALARMAS_MAX 4
#define svc_PERIODO_ALARMA 20

typedef void (*svc_alarma_callback_t)(EVENTO_T ID_evento,uint32_t auxData);

void svc_alarma_iniciar(svc_alarma_callback_t cb_a_llamar);

uint32_t svc_alarma_codificar(bool periodico,Tiempo_ms_t retardo_ms,uint8_t flags);

void svc_alarma_activar(uint32_t alarma_flags,EVENTO_T ID_evento,uint32_t auxData);

void svc_alarma_actualizar(EVENTO_T evento,uint32_t aux);
#endif