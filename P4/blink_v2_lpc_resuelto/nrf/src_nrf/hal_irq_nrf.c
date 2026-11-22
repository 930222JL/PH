#include "hal_irq.h"
#include <nrf.h>

uint32_t hal_irq_estado(void){
	return __get_PRIMASK();
}

void hal_irq_desactivar(void){
	__disable_irq();
}

void hal_irq_activar(uint32_t estado){
	__set_PRIMASK(estado);
}