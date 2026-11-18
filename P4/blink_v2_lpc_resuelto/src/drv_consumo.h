#ifndef DRV_CONSUMO_H
#define DRV_CONSUMO_H
#include <stdint.h>

void drv_consumo_iniciar(uint32_t id);
	
void drv_consumo_esperar(void);
	
void drv_consumo_dormir(void);

#endif