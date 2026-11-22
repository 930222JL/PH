#include "board.h"
#include <nrf.h>
#include <stdint.h>
#include "hal_gpio.h"
#include "hal_consumo.h"

static volatile uint32_t leds[LEDS_NUMBER]=LEDS_LIST;

void hal_consumo_iniciar(void){}
	
void hal_consumo_esperar(void){
	__wfi();
}
	
void hal_consumo_dormir(void){
	for(uint16_t i=0;i<LEDS_NUMBER;i++){
		hal_gpio_escribir(leds[i],1);
	}
	NRF_POWER->SYSTEMOFF=POWER_SYSTEMOFF_SYSTEMOFF_Enter;
	
	while(1){
		__wfe();
	}
}
