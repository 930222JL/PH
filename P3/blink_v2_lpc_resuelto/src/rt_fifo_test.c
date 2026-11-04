#include "rt_fifo.h"
#include "rt_fifo_test.h"
#include "board.h"
#include <stdint.h>

static uint32_t test_circular(void){
	EVENTO_T id;
	uint32_t data;
	Tiempo_us_t TS;
	
	Tiempo_us_t t_anterior;
	
	rt_FIFO_inicializar(MONITOR2);
	
	for(uint16_t i=0;i<5*RT_FIFO_TAM;i++){
		rt_FIFO_encolar(ev_PULSAR_BOTON,i);
		if(rt_FIFO_extraer(&id,&data,&TS) !=1)return 0;
		if(id!=ev_PULSAR_BOTON || data!=i)return 0;
		if(TS<t_anterior) return 0;
		t_anterior=TS;
	}
	
	if(rt_FIFO_estadisticas(ev_VOID)!=5*RT_FIFO_TAM) return 0;
	if(rt_FIFO_estadisticas(ev_PULSAR_BOTON)!=5*RT_FIFO_TAM) return 0;
	
	for(uint16_t i=0;i<RT_FIFO_TAM;i++){
		rt_FIFO_encolar(ev_PULSAR_BOTON,i);
	}
	
	for(uint16_t i=0;i<RT_FIFO_TAM;i++){
		if(rt_FIFO_extraer(&id,&data,&TS)!=RT_FIFO_TAM-i) return 0;
		if(data!=i) return 0;
	}
	
	if(rt_FIFO_extraer(&id,&data,&TS)!=0) return 0;
	return 1;
}

static uint32_t test_overflow(void){
	rt_FIFO_inicializar(MONITOR2);
	for(uint16_t i=0;i<=RT_FIFO_TAM;i++){
		rt_FIFO_encolar(ev_PULSAR_BOTON,i);
	}
	
	return 0;
}

void test(void){
	uint16_t ok=test_circular();
	ok=test_overflow();
}
	
	