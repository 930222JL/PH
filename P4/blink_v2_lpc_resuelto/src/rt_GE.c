#include "rt_GE.h"
#include "svc_GE.h"
#include "rt_fifo.h"
#include "rt_evento_t.h"
#include "drv_consumo.h"
#include "drv_monitor.h"
#include "board.h"
#include <stddef.h>

typedef struct{
	svc_GE_callback_t f;
	uint8_t prioridad;
} suscripcion_t;

static volatile suscripcion_t suscripciones[EVENT_TYPES][rt_GE_MAX_SUSCRITOS];

void overflow(){
	drv_monitor_marcar(MONITOR4);
	while(1);
}

void rt_GE_iniciar(){
	for(uint16_t i=0;i<EVENT_TYPES;i++){
			for(uint16_t j=0;j<rt_GE_MAX_SUSCRITOS;j++){
				suscripciones[i][j].f=NULL;
				suscripciones[i][j].prioridad=0;
			}
	}
}

void rt_GE_lanzador(){
	EVENTO_T id;
	uint32_t data;
	Tiempo_us_t t;
	
	while(1){
		if(rt_FIFO_extraer(&id,&data,&t)){
			for(uint16_t i=0;i<rt_GE_MAX_SUSCRITOS;i++){
				svc_GE_callback_t f=suscripciones[id][i].f;
				if(f==NULL) break;
				f(id,data);
			}
		}else{
			drv_consumo_esperar();
		}
	}
}

void rt_GE_actualizar(EVENTO_T ID_evento, uint32_t auxiliar){
	switch(ID_evento){
		case ev_usuario:
			
		case ev_INACTIVIDAD:
			drv_consumo_dormir();
	}
}

//Modulo svc

void svc_GE_suscribir(EVENTO_T ID_evento, uint8_t prioridad, svc_GE_callback_t f_callback){
	if(ID_evento == ev_VOID || ID_evento >= EVENT_TYPES || f_callback == 0) return;
	uint16_t n=0;
	while(n<rt_GE_MAX_SUSCRITOS && suscripciones[ID_evento][n].f!=NULL){
		if(suscripciones[ID_evento][n].f==f_callback)return;
		n++;
	}
	if(n==rt_GE_MAX_SUSCRITOS) overflow();
	uint16_t i=n;
	
	while(i>0 && suscripciones[ID_evento][i].prioridad>prioridad){
		suscripciones[ID_evento][i]=suscripciones[ID_evento][i-1];
		i--;
	}
	suscripciones[ID_evento][i].prioridad=prioridad;
	suscripciones[ID_evento][i].f=f_callback;
}

void svc_GE_cancelar(EVENTO_T ID_evento, svc_GE_callback_t f_callback){
	if(ID_evento == ev_VOID || ID_evento >= EVENT_TYPES || f_callback == 0) return;
	uint16_t n=0;
	while(n<rt_GE_MAX_SUSCRITOS && suscripciones[ID_evento][n].f!=f_callback){
		n++;
	}
	if(n==rt_GE_MAX_SUSCRITOS)return;
	
	while(n<rt_GE_MAX_SUSCRITOS && suscripciones[ID_evento][n].f!=NULL){
		suscripciones[ID_evento][n]=suscripciones[ID_evento][n+1];
		n++;
	}
}
	
	