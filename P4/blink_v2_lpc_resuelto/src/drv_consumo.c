#include "drv_consumo.h"
#include "hal_consumo.h"
#include "svc_diagnostico.h"
#include <stdint.h>

static volatile uint32_t monitor;

void drv_consumo_iniciar(void){
	monitor= svc_diagnostico_reservar();
	hal_consumo_iniciar();
}
	
void drv_consumo_esperar(void){
	hal_consumo_esperar();
}
	
void drv_consumo_dormir(void){
	hal_consumo_dormir();
}