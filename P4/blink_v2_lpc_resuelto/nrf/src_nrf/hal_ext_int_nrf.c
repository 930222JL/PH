#include "hal_ext_int.h"
#include "rt_evento_t.h"
#include "board.h"
#include <nrf.h>
#include <stddef.h>
#define NUM_CHANNELS 8

 #define GPIOTE_PUERTO(pin) (((pin) >> 5) & 1) //compruebo el puerto (nº de pin en los 5 bits bajos, puerto en los siguientes)
 #define GPIOTE_BIT(pin) ((pin) & 0x1Fu)										//cojo el bit(bits bajos)

static uint32_t canales[NUM_CHANNELS];	// Mapa canal GPIOTE -> pin asignado 
static uint32_t botones[BUTTONS_NUMBER]=BUTTONS_LIST;	// Pines de botones definidos en board.h 
static void (*f_cb)(EVENTO_T ID_evento,uint32_t pin);	// Callback al driver superior 

/* Inicializa el modulo GPIOTE, configura los botones de la placa y habilita su IRQ en NVIC. */
void hal_ext_int_iniciar(void (*f_callback_isr)(EVENTO_T ID_evento,uint32_t pin)){
	f_cb=f_callback_isr;
	NRF_GPIOTE->INTENCLR=0xFFFFFFFF;
	for(uint16_t i=0;i<NUM_CHANNELS;i++)canales[i]=0;
	for(uint16_t i=0;i<BUTTONS_NUMBER;i++)hal_ext_int_config_pin(botones[i]);
	NVIC_ClearPendingIRQ(GPIOTE_IRQn);
	NVIC_EnableIRQ(GPIOTE_IRQn);
}

/* Asigna un pin a un canal GPIOTE libre y lo configura como entrada pull-up por flanco de bajada. */
void hal_ext_int_config_pin(uint32_t pin){
	uint16_t n=0;
	while(n<NUM_CHANNELS){			// Comprueba si el pin ya tiene un canal asignado 
		if(canales[n]==pin)break;
		n++;
	}
	if(n!=NUM_CHANNELS)return;		// Busca el primer canal libre (valor 0) 
	n=0;
	while(n<NUM_CHANNELS){			
		if(canales[n]==0)break;
		n++;
	}
	if(n==NUM_CHANNELS)return;		// No quedan canales libres 
	canales[n]=pin;
	NRF_GPIOTE->INTENCLR=(1<<n);
	/* Configura el pin fisico en el bloque GPIO: entrada conectada con resistencia pull-up */
	NRF_GPIO->PIN_CNF[pin]=
		(GPIO_PIN_CNF_DIR_Input 		<< GPIO_PIN_CNF_DIR_Pos)		|
		(GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos)	|
		(GPIO_PIN_CNF_PULL_Pullup		<< GPIO_PIN_CNF_PULL_Pos)		|
		(GPIO_PIN_CNF_SENSE_Disabled<< GPIO_PIN_CNF_SENSE_Pos);
	/* Configura el canal GPIOTE en modo Evento por flanco de bajada */
	NRF_GPIOTE->CONFIG[n]=
		(GPIOTE_CONFIG_MODE_Event      << GPIOTE_CONFIG_MODE_Pos) |		//configura el pin como un input en IN[n]
		(GPIOTE_BIT(pin)               << GPIOTE_CONFIG_PSEL_Pos) |		//configura el bit del pint
		(GPIOTE_PUERTO(pin)            << GPIOTE_CONFIG_PORT_Pos) |		//configura el puerto del pin
		(GPIOTE_CONFIG_POLARITY_HiToLo << GPIOTE_CONFIG_POLARITY_Pos);//configura que cuando se pulse se limpie de CRL[n] i se genere evento en IN[n]
	NRF_GPIOTE->EVENTS_IN[n]=0;
	hal_ext_int_habilitar(pin);
}

/* Deshabilita la interrupcion del canal asociado al pin indicado. */
void hal_ext_int_deshabilitar(uint32_t pin){
	uint16_t n=0;
	while(n<NUM_CHANNELS){
		if(canales[n]==pin)break;
		n++;
	}
	if(n==NUM_CHANNELS)return;
	NRF_GPIOTE->INTENCLR=(1<<n);
}

/* Limpia eventos previos y habilita la interrupcion del canal asociado al pin. */
void hal_ext_int_habilitar(uint32_t pin){
	uint16_t n=0;
	while(n<NUM_CHANNELS){
		if(canales[n]==pin)break;
		n++;
	}
	if(n==NUM_CHANNELS)return;
	NRF_GPIOTE->EVENTS_IN[n]=0;
	NRF_GPIOTE->INTENSET=(1<<n);
}

/* Rutina de atencion a la interrupcion (ISR) compartida por todos los canales GPIOTE. */
void GPIOTE_IRQHandler(void){
	for(uint16_t i=0;i<NUM_CHANNELS;i++){
		/* Comprueba si el canal i genero evento y tiene su interrupcion habilitada */
		if(NRF_GPIOTE->EVENTS_IN[i] && (NRF_GPIOTE->INTENSET &(1<<i))){
			NRF_GPIOTE->EVENTS_IN[i]=0;
			if(canales[i]!=0 && f_cb!=NULL) f_cb(ev_PULSAR_BOTON,canales[i]);
		}
	}
}

