#include "rt_fifo.h"
#include "drv_tiempo.h"
#include "drv_monitor.h"

typedef struct{
	EVENTO_T ID_evento;
	uint32_t auxData;
	Tiempo_us_t TS;
} EVENTO;

static volatile uint16_t siguiente_a_tratar, ultimo_tratado,pendientes,total_eventos;

static volatile uint32_t s_monitor;
EVENTO rt_FIFO[RT_FIFO_TAM];		//cola de eventos
uint32_t total_por_evento[EVENT_TYPES];		//vector para contar cuantas veces aparece cada tipo de evento

void rt_FIFO_inicializar(uint32_t monitor_overflow){
	s_monitor=monitor_overflow;
	siguiente_a_tratar=ultimo_tratado=pendientes=total_eventos=0;
	for(uint16_t i=0; i<EVENT_TYPES; i++) total_por_evento[i]=0;
	for(uint16_t i=0; i<RT_FIFO_TAM; i++){
		rt_FIFO[i].ID_evento=ev_VOID;
		rt_FIFO[i].auxData=0;
		rt_FIFO[i].TS=0;
	}
}

void rt_FIFO_encolar(uint32_t ID_evento, uint32_t auxData){
	if(pendientes>=RT_FIFO_TAM){
		drv_monitor_marcar(s_monitor);
		while(1);
	}
	rt_FIFO[siguiente_a_tratar].ID_evento=ID_evento;
	rt_FIFO[siguiente_a_tratar].auxData=auxData;
	rt_FIFO[siguiente_a_tratar].TS=drv_tiempo_actual_us();
	siguiente_a_tratar=(siguiente_a_tratar+1)%RT_FIFO_TAM;
	pendientes++;
	total_eventos++;
	if(ID_evento<EVENT_TYPES)total_por_evento[ID_evento]++;
}

uint8_t rt_FIFO_extraer(EVENTO_T *ID_evento,uint32_t *auxData,Tiempo_us_t *TS){
	if(pendientes>0){
		*ID_evento=rt_FIFO[siguiente_a_tratar].ID_evento;
		*auxData=rt_FIFO[siguiente_a_tratar].auxData;
		*TS=rt_FIFO[siguiente_a_tratar].TS;
		ultimo_tratado=(ultimo_tratado+1)%RT_FIFO_TAM;
		pendientes--;
	}
	return pendientes;
}

uint32_t rt_FIFO_estadisticas(EVENTO_T ID_evento){
	if(ID_evento==ev_VOID)return total_eventos;
	if(ID_evento<EVENT_TYPES)return total_por_evento[ID_evento];
	return 0;
}
	
	