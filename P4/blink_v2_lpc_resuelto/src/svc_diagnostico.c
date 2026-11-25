#include "svc_diagnostico.h"
#include "drv_monitor.h"
#include "board.h"

#if MONITOR_NUMBER <2
#error "deben haber al menos dos monitores para que tenga sentido el uso de un servicio de gestion de monitores"
#endif

#define MONITOR_ERROR MONITOR1

uint32_t monitores[MONITOR_NUMBER]=MONITOR_LIST;	/* Lista de pines fisicos de monitorizacion */
uint16_t reservados;	/* Indice del siguiente monitor disponible */

/* Inicializa el driver de monitores y reserva el indice 0 para MONITOR_ERROR */
err_t svc_diagnostico_iniciar(void){
	drv_monitor_iniciar();
	reservados=1;
	return OK;
}

/* Asigna el siguiente monitor disponible a un modulo solicitante */
CANAL_T svc_diagnostico_reservar(void){
	if(reservados>=MONITOR_NUMBER){
		svc_diagnostico_error();	/* No quedan monitores libres: activa LED de error */
		return 0;
	}
	uint32_t monitor=monitores[reservados];
	reservados++;
	return monitor;
}

/* Activa (pone a nivel activo) el pin del canal de diagnostico indicado */
err_t svc_diagnostico_senyalar(CANAL_T canal){
	if(canal>=monitores[reservados])return CANAL_INVALIDO;
	drv_monitor_marcar(canal);
	return OK;
}

/* Desactiva el pin del canal de diagnostico indicado */
err_t svc_diagnostico_limpiar(CANAL_T canal){
	if(canal>=monitores[reservados])return CANAL_INVALIDO;
	drv_monitor_desmarcar(canal);
	return OK;
}

/* Activa el indicador visual de error del sistema */
void svc_diagnostico_error(void){
	drv_monitor_marcar(MONITOR_ERROR);
}
