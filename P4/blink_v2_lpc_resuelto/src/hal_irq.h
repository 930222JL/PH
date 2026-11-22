#ifndef HAL_IRQ_H
#define HAL_IRQ_H
#include <stdint.h>

uint32_t hal_irq_estado(void);

void hal_irq_desactivar(void);

void hal_irq_activar(uint32_t);

#endif