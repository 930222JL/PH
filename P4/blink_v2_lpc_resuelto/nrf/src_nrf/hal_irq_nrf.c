#include "hal_irq.h"
#include <nrf.h>

/* Devuelve el estado actual de la mascara de interrupciones (PRIMASK).
 * 0 = interrupciones habilitadas, 1 = deshabilitadas. */
uint32_t hal_irq_estado(void){
	return __get_PRIMASK();
}

/* Deshabilita las interrupciones globales enmascarables. */
void hal_irq_desactivar(void){
	__disable_irq();
}

/* Restaura el estado previo de las interrupciones globales. */
void hal_irq_activar(uint32_t estado){
	__set_PRIMASK(estado);
}
