#include "svc_diagnostico.h"
#include "drv_monitor.h"
#include "board.h"

#if MONITOR_NUMBER <2
#error "deben haber al menos dos monitores para que tenga sentido el uso de un servicio de gestion de monitores"
#endif

#define MONITOR_ERROR MONITOR1

uint32_t monitores[MONITOR_NUMBER]=MONITOR_LIST;
uint16_t reservados;

err_t svc_diagnostico_iniciar(void){
	drv_monitor_iniciar();
	reservados=1;
	return OK;
}

CANAL_T svc_diagnostico_reservar(void){
	if(reservados>=MONITOR_NUMBER){
		svc_diagnostico_error();
		return 0;
	}
	uint32_t monitor=monitores[reservados];
	reservados++;
	return monitor;
}

err_t svc_diagnostico_senyalar(CANAL_T canal){
	if(canal>=monitores[reservados])return CANAL_INVALIDO;
	drv_monitor_marcar(canal);
	return OK;
}

err_t svc_diagnostico_limpiar(CANAL_T canal){
	if(canal>=monitores[reservados])return CANAL_INVALIDO;
	drv_monitor_desmarcar(canal);
	return OK;
}

void svc_diagnostico_error(void){
	drv_monitor_marcar(MONITOR_ERROR);
}
