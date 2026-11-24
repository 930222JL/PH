#include "hal_consumo.h"
#include <LPC210x.H>
#include "board.h" 
#include <stdint.h>
#include "drv_leds.h"

static uint32_t leds[LEDS_NUMBER]=LEDS_LIST;

void hal_consumo_iniciar(void){}
	
void hal_consumo_esperar(void){
	PCON=(PCON | (1<<0));		//con un 1 en el bit 0 de PCON el procesador se duerme hasta que haya una interrupcion
}
	
void hal_consumo_dormir(void){
	for(uint16_t i=0;i<LEDS_NUMBER;i++){
		drv_led_establecer(leds[i],LED_OFF);
	}
	PCON=(PCON | (1<<1));		//con un 1 en el bit 0 de PCON el procesador se duerme más profundo hasta que haya una interrupción externa
}
