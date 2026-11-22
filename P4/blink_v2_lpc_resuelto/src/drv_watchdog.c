#include "drv_watchdog.h"
#include "hal_wdt.h"

void drv_watchdog_iniciar(uint32_t timeout_ms){
	hal_wdt_iniciar(timeout_ms);
}

void drv_watchdog_alimentar(void){
	hal_wdt_alimentar();
}

void drv_watchdog_probar_bloqueo(void){
	while(1){ /* no se alimenta: el watchdog debe resetear el sistema */ }
}
