#include <nrf.h>
#include "hal_consumo.h"

void hal_consumo_iniciar(void){}
	
void hal_consumo_esperar(void){
	__wfi();
}
	
void hal_consumo_dormir(void){
	__wfi();
}