/* *****************************************************************************
 * hal_wdt_nrf.c: watchdog del nRF52840 (periferico WDT, reloj de 32768 Hz).
 * ****************************************************************************/
#include <nrf.h>
#include <stdint.h>
#include "hal_wdt.h"

#define WDT_FRECUENCIA_HZ   32768u
#define WDT_CRV_MINIMO      15u      /* minimo permitido por el hardware */
#define WDT_CANAL_RR        0u       /* canal de recarga utilizado */

/* Inicializa y arranca el Watchdog con un tiempo limite en milisegundos. */
/* Calcula los ciclos de reloj necesarios usando aritmetica de 64 bits para evitar overflow */
void hal_wdt_iniciar(uint32_t timeout_ms){
	uint32_t crv = (uint32_t)(((uint64_t)timeout_ms * WDT_FRECUENCIA_HZ) / 1000u);
	if(crv > 0) crv--;                       /* timeout = (CRV + 1) / 32768 s */
	if(crv < WDT_CRV_MINIMO) crv = WDT_CRV_MINIMO;

	/* SLEEP_Run: sigue contando con la CPU en WFI (modo esperar).
	 * HALT_Pause: se pausa si se detiene la CPU con el depurador. */
	NRF_WDT->CONFIG = (WDT_CONFIG_SLEEP_Run  << WDT_CONFIG_SLEEP_Pos) |
	                  (WDT_CONFIG_HALT_Pause << WDT_CONFIG_HALT_Pos);

	NRF_WDT->CRV    = crv;	/* Carga el valor de cuenta */
	NRF_WDT->RREN   = (WDT_RREN_RR0_Enabled << WDT_RREN_RR0_Pos);  /* habilita canal 0 */
	NRF_WDT->TASKS_START = 1;                                      /* arranca el contador */
}

/* Alimenta (resetea la cuenta) del Watchdog escribiendo la clave especial de recarga. */
void hal_wdt_alimentar(void){
	NRF_WDT->RR[WDT_CANAL_RR] = WDT_RR_RR_Reload;   /* 0x6E524635 */
}
