#include "rt_GE.h"
#include "svc_GE.h"
#include "svc_alarmas.h"
#include "rt_fifo.h"
#include "rt_evento_t.h"
#include "drv_consumo.h"
#include "drv_watchdog.h"
#include "drv_monitor.h"
#include "hal_irq.h"
#include "board.h"
#include <stddef.h>

/* Entrada en la tabla de suscripciones: funcion callback y su prioridad */
typedef struct{
	svc_GE_callback_t f;
	uint8_t prioridad;
} suscripcion_t;

/* Matriz bidimensional [Tipo de Evento][Suscriptor] */
static volatile suscripcion_t suscripciones[EVENT_TYPES][rt_GE_MAX_SUSCRITOS];

/* Bloquea el sistema encendiendo el monitor 4 en caso de desbordar suscriptores */
static void overflow(){
	drv_monitor_marcar(MONITOR4);
	while(1);
}

/* Limpia la tabla de suscripciones y registra el propio gestor para eventos de actividad */
void rt_GE_iniciar(void){
	for(uint16_t i=0;i<EVENT_TYPES;i++){
			for(uint16_t j=0;j<rt_GE_MAX_SUSCRITOS;j++){
				suscripciones[i][j].f=NULL;
				suscripciones[i][j].prioridad=0;
			}
	}
	svc_GE_suscribir(ev_INACTIVIDAD,0,rt_GE_actualizar);
	svc_GE_suscribir(ev_PULSAR_BOTON,0,rt_GE_actualizar);
}

/* Bucle infinito principal (planificador cooperativo dirigido por eventos) */
void rt_GE_lanzador(void){
	EVENTO_T id;
	uint32_t data;
	Tiempo_us_t t;
	/* Programa alarma inicial de inactividad a 15 segundos */
	svc_alarma_activar(svc_alarma_codificar(false,15000,0),ev_INACTIVIDAD,0);
	
	while(1){
		/* Extrae el siguiente evento de la cola FIFO */
		if(rt_FIFO_extraer(&id,&data,&t)){
			/* Notifica a todos los modulos suscritos a ese evento */
			for(uint16_t i=0;i<rt_GE_MAX_SUSCRITOS;i++){
				svc_GE_callback_t f=suscripciones[id][i].f;
				if(f==NULL) break;
				f(id,data);
			}
			drv_watchdog_alimentar();	/* Refresca el watchdog tras procesar actividad */
		}else{
			/* Si no hay eventos pendientes, pone la CPU en modo espera (WFI) */
			drv_consumo_esperar();
		}
	}
}

/* Gestiona el temporizador de inactividad y el paso a modo dormir profundo */
void rt_GE_actualizar(EVENTO_T ID_evento, uint32_t auxiliar){
	switch(ID_evento){
		case ev_INACTIVIDAD:
			drv_consumo_dormir();
		default:
			/* Rearma la alarma de inactividad a 10 segundos */
			svc_alarma_activar(svc_alarma_codificar(false,10000,0),ev_INACTIVIDAD,0);
	}
}

//Modulo svc

/* Suscribe un callback a un evento, insertandolo ordenado por prioridad (menor numero = mayor prioridad) */
void svc_GE_suscribir(EVENTO_T ID_evento, uint8_t prioridad, svc_GE_callback_t f_callback){
	if(ID_evento == ev_VOID || ID_evento >= EVENT_TYPES || f_callback == 0) return;
	uint16_t n=0;
	/* Busca el final de la lista y evita suscripciones duplicadas */
	while(n<rt_GE_MAX_SUSCRITOS && suscripciones[ID_evento][n].f!=NULL){
		if(suscripciones[ID_evento][n].f==f_callback)return;
		n++;
	}
	if(n==rt_GE_MAX_SUSCRITOS) overflow();
	uint16_t i=n;

	/* Desplaza elementos para mantener el orden por prioridad */
	while(i>0 && suscripciones[ID_evento][i].prioridad>prioridad){
		suscripciones[ID_evento][i]=suscripciones[ID_evento][i-1];
		i--;
	}
	suscripciones[ID_evento][i].prioridad=prioridad;
	suscripciones[ID_evento][i].f=f_callback;
}

/* Elimina una suscripcion y compacta el array hacia la izquierda para no dejar huecos */
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

	
