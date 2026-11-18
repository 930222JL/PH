#include "drv_consumo.h"
#include "hal_consumo.h"
#include "drv_monitor.h"
#include <stdint.h>

static volatile uint32_t s_monitor;


void drv_consumo_iniciar(uint32_t id){
	s_monitor=id;
	hal_consumo_iniciar();
	drv_monitor_marcar(s_monitor);
}
	
void drv_consumo_esperar(void){
	drv_monitor_desmarcar(s_monitor);
	hal_consumo_esperar();
	drv_monitor_marcar(s_monitor);
}
	
void drv_consumo_dormir(void){
	hal_consumo_dormir();
}
