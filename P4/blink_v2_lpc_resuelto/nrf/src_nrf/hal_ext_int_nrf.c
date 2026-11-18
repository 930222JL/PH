#include "hal_ext_int.h"
#include <nrf.h>
#define NUM_CHANNELS 8

 #define GPIOTE_PUERTO(pin) (((pin) >> 5) ? NRF_P1:NRF_P0) //compruebo el puerto (nº de pin en los 5 bits bajos, puerto en los siguientes)
 #define GIPOTE_BIT(pin) ((pin) & 0x1Fu)										//cojo el bit(bits bajos)

uint32_t canales[NUM_CHANNELS];
static volatile void (*f_cb)(uint32_t pin);

void hal_ext_int_iniciar(void (*f_callback_isr)(uint32_t pin)){
	f_cb=f_callback_isr;
	for(uint16_t i=0;i<NUM_CHANNELS;i++)canales[i]=0;
	NRF_GPIOTE->INTENCLR=0xFFFFFFFF;
	NVIC_CLEAR_PENDING_IRQ(GPIOTE_IRQn);
	NVIC_ENABLE_IRQ(GPIOTE_IRQn);
}

void hal_ext_int_config_pin(uint32_t pin){
	uint16_t n=0;
	while(n<NUM_CHANNELS){
		if(canales[n]==pin)break;
		n++;
	}
	if(n!=NUM_CHANNELS)return;
	n=0;
	while(n<NUM_CHANNELS){
		if(canales[n]==0)break;
		n++;
	}
	if(n==NUM_CHANNELS)return;
	canales[n]=pin;
	NRF_GPIOTE->INTENCLR=(1<<n);
	NRF_GPIOTE->CONFIG[n]=
		(GPIOTE_CONFIG_MODE_Event      << GPIOTE_CONFIG_MODE_Pos) |		//configura el pin como un input en IN[n]
		(GPIOTE_BIT                    << GPIOTE_CONFIG_PSEL_Pos) |		//configura el bit del pint
		(GPIOTE_PUERTO                 << GPIOTE_CONFIG_PORT_Pos) |		//configura el puerto del pin
		(GPIOTE_CONFIG_POLARITY_HiToLo << GPIOTE_CONFIG_POLARITY_Pos);//configura que cuando se pulse se limpie de CRL[n] i se genere evento en IN[n]
	NRF_GPIOTE->EVENTS_IN[n]=0;
}

void hal_ext_int_deshabilitar(uint32_t pin){
	uint16_t n=0;
	while(n<NUM_CHANNELS){
		if(canales[n]==pin)break;
		n++;
	}
	if(n==NUM_CHANNELS)return;
	NRF_GPIOTE->INTENCLR=(1<<n);
}

void hal_ext_int_habilitar(uint32_t pin){
	uint16_t n=0;
	while(n<NUM_CHANNELS){
		if(canales[n]==pin)break;
		n++;
	}
	if(n==NUM_CHANNELS)return;
	NRF_GPIOTE->INTENSET=(1<<n);
}