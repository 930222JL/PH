#include "rt_fifo.h"
#include <stddef.h>
#include <nrf.h>
#include "drv_tiempo.h"
#include "drv_monitor.h"
#include "board.h"      

typedef struct{
	EVENTO_T ID_evento;
	uint32_t auxData;
	Tiempo_us_t TS;
} EVENTO;


static volatile uint16_t siguiente_a_escribir, siguiente_a_tratar, pendientes, total_eventos;

static volatile uint32_t s_monitor;
EVENTO rt_FIFO[RT_FIFO_TAM];                 
uint32_t total_por_evento[EVENT_TYPES];     

void rt_FIFO_inicializar(uint32_t monitor_overflow){
	s_monitor = monitor_overflow;
	drv_monitor_desmarcar(s_monitor);
	siguiente_a_escribir = siguiente_a_tratar = pendientes = total_eventos = 0;
	for(uint16_t i=0; i<EVENT_TYPES; i++) total_por_evento[i] = 0;
	for(uint16_t i=0; i<RT_FIFO_TAM; i++){
		rt_FIFO[i].ID_evento = ev_VOID;
		rt_FIFO[i].auxData = 0;
		rt_FIFO[i].TS = 0;
	}
}

void rt_FIFO_encolar(EVENTO_T ID_evento, uint32_t auxData){
	uint32_t primask = __get_PRIMASK();
	__disable_irq();                          /* se llama desde ISR y desde main */

	if(pendientes >= RT_FIFO_TAM){            
		drv_monitor_marcar(s_monitor);
		while(1);
	}
	rt_FIFO[siguiente_a_escribir].ID_evento = ID_evento;
	rt_FIFO[siguiente_a_escribir].auxData = auxData;
	rt_FIFO[siguiente_a_escribir].TS = drv_tiempo_actual_us();
	siguiente_a_escribir = (siguiente_a_escribir + 1) % RT_FIFO_TAM;
	pendientes++;
	total_eventos++;
	if(ID_evento < EVENT_TYPES) total_por_evento[ID_evento]++;

	__set_PRIMASK(primask);
}


uint8_t rt_FIFO_extraer(EVENTO_T *ID_evento, uint32_t *auxData, Tiempo_us_t *TS){
	uint32_t primask = __get_PRIMASK();
	uint8_t hay;
	__disable_irq();

	hay = (uint8_t)pendientes;
	if(pendientes > 0){
		*ID_evento = rt_FIFO[siguiente_a_tratar].ID_evento;
		*auxData   = rt_FIFO[siguiente_a_tratar].auxData;
		*TS        = rt_FIFO[siguiente_a_tratar].TS;
		siguiente_a_tratar = (siguiente_a_tratar + 1) % RT_FIFO_TAM;
		pendientes--;
	}

	__set_PRIMASK(primask);
	return hay;
}

uint32_t rt_FIFO_estadisticas(EVENTO_T ID_evento){
	if(ID_evento == ev_VOID) return total_eventos;
	if(ID_evento < EVENT_TYPES) return total_por_evento[ID_evento];
	return 0;
}