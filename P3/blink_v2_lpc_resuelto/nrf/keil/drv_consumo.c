#include "drv_consumo.h"


void drv_consumo_iniciar(void){
	hal_consumo_iniciar();
}
	
void drv_consumo_esperar(void){
	hal_consumo_esperar();
}
	
void drv_consumo_dormir(void){
	hal_consumo_dormir();
}