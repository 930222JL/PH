#ifndef SVC_GE_H 
#define SVC_GE_H
#include "rt_evento_t.h"
#include <stdint.h>

typedef void (*svc_GE_callback_t)(EVENTO_T ID_evento,uint32_t auxData);

void svc_GE_suscribir(EVENTO_T ID_evento,uint8_t prioridad,void (*f_callback)());

void svc_GE_cancelar(EVENTO_T ID_evento,void (*f_callback)());

#endif
