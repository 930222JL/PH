#include "drv_monitor.h"
#include "board.h"
#include "hal_gpio.h"

static volatile uint32_t s_monitores[MONITOR_NUMBER] = MONITOR_LIST;

void drv_monitor_iniciar(void){
	for(uint16_t i=0;i<MONITOR_NUMBER;i++){
		hal_gpio_sentido(s_monitores[i],HAL_GPIO_PIN_DIR_OUTPUT);
		hal_gpio_escribir(s_monitores[i],0);
	}
}

void drv_monitor_marcar(uint32_t id){
	int i;
	switch(id){
		case MONITOR1:
			i=0;
		case MONITOR2:
			i=1;
		case MONITOR3:
			i=2;
		case MONITOR4:
			i=3;
	}
	hal_gpio_escribir(s_monitores[i],1);
}

void drv_monitor_desmarcar(uint32_t id){
	int i;
	switch(id){
		case MONITOR1:
			i=0;
		case MONITOR2:
			i=1;
		case MONITOR3:
			i=2;
		case MONITOR4:
			i=3;
	}
	hal_gpio_escribir(s_monitores[i],0);
}
