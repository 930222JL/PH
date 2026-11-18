#include <nrf.h>
#include <stdint.h>
#include "drv_leds.c"
#include "hal_consumo.h"

static volatile uint32_t leds[LEDS_NUMBER]=LEDS_LIST

void hal_consumo_iniciar(void){}
	
void hal_consumo_esperar(void){
	__wfi();
}
	
void hal_consumo_dormir(void){
	for(uint16_t i=0;i<LEDS_NUMBER;i++){
		drv_led_establecer(leds[i],0);
	}
	NRF_POWER->SYSTEM_OFF=POWER_SYSTEMOFF_SYSTEMOFF_Enter;
	
	while(1){
		__wfe();
}
