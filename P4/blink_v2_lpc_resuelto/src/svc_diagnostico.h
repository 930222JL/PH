#ifndef SVC_DIAGNOSTICO_H 
#define SVC_DIAGNOSTICO_H
#include <stdint.h>

typedef enum{
	OK,
	SIN_CANALES,
	CANAL_INVALIDO,
} err_t;

typedef uint32_t CANAL_T;

err_t svc_diagnostico_iniciar(void);

CANAL_T svc_diagnostico_reservar(void);

err_t svc_diagnostico_senyalar(CANAL_T canal);

err_t svc_diagnostico_limpiar(CANAL_T canal);

void svc_diagnostico_error(void);

#endif
