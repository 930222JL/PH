#include "hal_ext_int.h"
#include "hal_gpio.h"
#include "drv_botones.h"
#include "rt_evento_t.h"
#include "svc_GE.h"
#include "svc_alarmas.h"
#include "board.h"
#include <stdbool.h>
#include <stdint.h>

#if defined(LPC2105_simulador)
#define BOTONES 3

#define BOTON_1	16
#define BOTON_2	14
#define BOTON_3	15

#define LISTA_BOTONES {BOTON_1, BOTON_2, BOTON_3}
#else
#define BOTONES 4

#define BOTON_1 BUTTON_1
#define BOTON_2	BUTTON_2
#define BOTON_3	BUTTON_3
#define BOTON_4	BUTTON_4

#define LISTA_BOTONES {BOTON_1, BOTON_2, BOTON_3, BOTON_4}

#endif

typedef struct{
	uint32_t pin;
	bool pulsado;
	bool interrupciones;
	bool salida;
} boton;

static void (*f_cb)(EVENTO_T ID_evento,uint32_t auxData);
static volatile boton botones[BOTONES];
static uint32_t pines[BOTONES]=LISTA_BOTONES;

void drv_botones_iniciar(void (*cb_a_llamar)(EVENTO_T ID_evento,uint32_t auxData)){
	f_cb=cb_a_llamar;
	for(uint32_t i=0;i<BOTONES;i++){
		botones[i].pin=pines[i];
	}
	for(uint16_t i=0;i<BOTONES;i++){
		botones[i].pulsado=false;
		botones[i].interrupciones=true;
		botones[i].salida=false;
	}
	hal_ext_int_iniciar(drv_botones_pulsar_boton);
	svc_GE_suscribir(ev_PULSAR_BOTON,0,drv_botones_actualizar);
	svc_GE_suscribir(ev_BOTON_MUESTREO,0,drv_botones_actualizar);
	svc_GE_suscribir(ev_BOTON_REBOTES,0,drv_botones_actualizar);
}

void drv_botones_pulsar_boton(EVENTO_T ID_evento,uint32_t pin){
	hal_ext_int_deshabilitar(pin);
	uint16_t n=0;
	while(n<BOTONES){
		if(botones[n].pin==pin)break;
		n++;
	}
	if(n<BOTONES){
		botones[n].pulsado=true;
		f_cb(ev_PULSAR_BOTON,n+1);
	}
}

void drv_botones_actualizar(EVENTO_T ID_evento,uint32_t auxData){
	switch(ID_evento){
		case ev_PULSAR_BOTON:
			for(uint16_t i=0;i<BOTONES;i++){
				if(botones[i].pulsado && botones[i].interrupciones){
					botones[i].interrupciones=false;
					svc_alarma_activar(svc_alarma_codificar(false,10,0),ev_BOTON_MUESTREO,0);
				}
			}
			break;
		
		case ev_BOTON_MUESTREO:
			for(uint16_t i=0;i<BOTONES;i++){
				if(botones[i].pulsado && !botones[i].interrupciones){
					if(hal_gpio_leer(botones[i].pin)){
						svc_alarma_activar(svc_alarma_codificar(true,10,0),ev_BOTON_MUESTREO,0);
					}else{
						svc_alarma_activar(0,ev_BOTON_MUESTREO,0);
						botones[i].salida=true;
						svc_alarma_activar(svc_alarma_codificar(false,10,0),ev_BOTON_REBOTES,0);
					}
				}
			}
			break;
			
		case ev_BOTON_REBOTES:
			for(uint16_t i=0;i<BOTONES;i++){
				if(botones[i].salida){
					botones[i].salida=false;
					botones[i].pulsado=false;
					botones[i].interrupciones=true;
					hal_ext_int_habilitar(botones[i].pin);
				}
			}
			break;
		
		default:
			break;
	}
}


		
			
			