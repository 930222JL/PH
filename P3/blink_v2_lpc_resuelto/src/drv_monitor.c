#include "drv_monitor.h"
#include "board.h"
#include "hal_gpio.h"

static volatile uint32_t s_monitores[MONITOR_NUMBER] = MONITOR_LIST;

void drv_monitor_iniciar(void){
	hal_gpio_iniciar();
	for(uint16_t i=0;i<MONITOR_NUMBER;i++){
		hal_gpio_sentido(s_monitores[i],HAL_GPIO_PIN_DIR_OUTPUT);
		hal_gpio_escribir(s_monitores[i],0);
	}
}

void drv_monitor_marcar(uint32_t id){
	if(id < MONITOR_NUMBER){
		hal_gpio_escribir(s_monitores[id],1);
	}
}

void drv_monitor_desmarcar(uint32_t id){
	if(id < MONITOR_NUMBER){
		hal_gpio_escribir(s_monitores[id],0);
	}
}