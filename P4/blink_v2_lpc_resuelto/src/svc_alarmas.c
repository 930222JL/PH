#include "svc_alarmas.h"
#include "drv_tiempo.h"
#include "drv_monitor.h"
#include "board.h"
#include "svc_GE.h"


#define ALARMA_PERIODICA_POS  31u
#define ALARMA_FLAGS_POS      24u
#define ALARMA_FLAGS_MASK     0x7Fu
#define ALARMA_RETARDO_MASK   0x00FFFFFFu  

typedef struct{
	bool activa;
	bool periodica;
	uint8_t flags;
	EVENTO_T ID_evento;
	uint32_t auxData;
	uint32_t tiempo;
	uint32_t contador;
}alarma;
	

static volatile svc_alarma_callback_t s_cb;
static volatile alarma s_alarmas[svc_ALARMAS_MAX];

void overflow(){
	drv_monitor_marcar(MONITOR4);
	while(1);
}

void svc_alarma_iniciar(svc_alarma_callback_t cb_a_llamar){
	s_cb=cb_a_llamar;
	for(uint16_t i=0;i<svc_ALARMAS_MAX;i++){
		s_alarmas[i].activa=false;
	}
	svc_GE_suscribir(ev_A_PERIODICA,0,svc_alarma_actualizar);
	drv_tiempo_periodico_ms(svc_PERIODO_ALARMA,s_cb,ev_A_PERIODICA);
}

uint32_t svc_alarma_codificar(bool periodico,Tiempo_ms_t retardo_ms,uint8_t flags){
	return ((uint32_t)(periodico ? 1u : 0u) << ALARMA_PERIODICA_POS)
	     | ((uint32_t)(flags & ALARMA_FLAGS_MASK) << ALARMA_FLAGS_POS)
	     | (retardo_ms & ALARMA_RETARDO_MASK);
}

void svc_alarma_activar(uint32_t alarma_flags,EVENTO_T ID_evento,uint32_t auxData){
	uint32_t tiempo=alarma_flags&ALARMA_RETARDO_MASK;
	bool activa= tiempo!=0;
	int libre=0;
	uint16_t n=0;
	while(n<svc_ALARMAS_MAX){
		if(s_alarmas[n].activa){
			if(s_alarmas[n].ID_evento==ID_evento){
				s_alarmas[n].activa=activa;
				s_alarmas[n].contador=0;
				s_alarmas[n].tiempo=tiempo/svc_PERIODO_ALARMA;
				s_alarmas[n].flags=(alarma_flags>>ALARMA_FLAGS_POS)&ALARMA_FLAGS_MASK;
				s_alarmas[n].periodica=alarma_flags >> ALARMA_FLAGS_POS;
				s_alarmas[n].auxData=auxData;
				return;
			}
		}else if(libre==0){
			libre=n;
		}
		n++;
	}
	if(n==svc_ALARMAS_MAX) overflow();
	s_alarmas[libre].ID_evento=ID_evento;
	s_alarmas[libre].activa=activa;
	s_alarmas[libre].tiempo=tiempo;
	s_alarmas[libre].contador=0;
	s_alarmas[libre].flags=(alarma_flags>>ALARMA_FLAGS_POS)&ALARMA_FLAGS_MASK;
	s_alarmas[libre].periodica=alarma_flags >> ALARMA_FLAGS_POS;
	s_alarmas[libre].auxData=auxData;
}

void svc_alarma_actualizar(EVENTO_T evento,uint32_t aux){
	for(uint16_t i=0;i<svc_ALARMAS_MAX;i++){
		if(!s_alarmas[i].activa)continue;
		 s_alarmas[i].contador++;
		if(s_alarmas[i].contador>=s_alarmas[i].tiempo){
			EVENTO_T id=s_alarmas[i].ID_evento;
			uint32_t data=s_alarmas[i].auxData;
			if(s_alarmas[i].periodica){
				s_alarmas[i].contador=0;
			}else{
				s_alarmas[i].activa=false;
			}
			if(s_cb) s_cb(id,data);
		}
	}
}
	
	
	
