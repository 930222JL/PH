#include "board.h"
#include <nrf.h>
#include <stdint.h>
#include "drv_leds.h"
#include "hal_consumo.h"

static volatile uint32_t leds[LEDS_NUMBER]={1,2,3,4};

void hal_consumo_iniciar(void){}
	
void hal_consumo_esperar(void){
	__wfi();
}
	
void hal_consumo_dormir(void){
	for(uint16_t i=0;i<LEDS_NUMBER;i++){
		drv_led_establecer(leds[i],LED_OFF);
	}
	NRF_POWER->SYSTEMOFF=POWER_SYSTEMOFF_SYSTEMOFF_Enter;
	
	while(1){
		__wfe();
	}
}
