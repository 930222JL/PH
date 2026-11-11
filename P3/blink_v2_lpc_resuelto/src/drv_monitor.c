#include "drv_monitor.h"
#include "board.h"
#include "hal_gpio.h"

// Array con la lista de pines GPIO asignados a cada monitor (inicializado con la macro MONITOR_LIST)
static volatile uint32_t s_monitores[MONITOR_NUMBER] = MONITOR_LIST;

// Inicializa los pines de los monitores configurándolos como salidas y poniéndolos a nivel bajo (0)
void drv_monitor_iniciar(void){
	for(uint16_t i=0;i<MONITOR_NUMBER;i++){
		hal_gpio_sentido(s_monitores[i],HAL_GPIO_PIN_DIR_OUTPUT); // Configura el pin del monitor i como salida
		hal_gpio_escribir(s_monitores[i],0);                      // Establece el pin a 0 (nivel bajo)
	}
}

// Activa (pone a nivel alto, 1) el pin asociado al identificador de monitor indicado
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
	hal_gpio_escribir(s_monitores[i],1); // Pone a 1 el GPIO correspondiente al monitor seleccionado
}

// Desactiva (pone a nivel bajo, 0) el pin asociado al identificador de monitor indicado
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
	hal_gpio_escribir(s_monitores[i],0); // Pone a 0 el GPIO correspondiente al monitor seleccionado
}
