/* *****************************************************************************
 * hal_wdt_lpc.c: watchdog del LPC2105.
 * El contador del WD cuenta a PCLK/4. Ajusta HAL_WDT_PCLK_HZ a la
 * frecuencia de PCLK de tu proyecto (ver system_/startup del simulador).
 * ****************************************************************************/
#include <LPC210x.h>
#include <stdint.h>
#include "hal_irq.h"
#include "hal_wdt.h"

#define HAL_WDT_PCLK_HZ     15000000u   /* <-- AJUSTAR a tu PCLK */
#define WDT_MINIMO_TICKS    0xFFu       /* valor minimo de WDTC */
#define WDMOD_WDEN          0x01u       /* habilita el watchdog */
#define WDMOD_WDRESET       0x02u       /* el timeout provoca reset */
#define WDFEED_PRIMERA      0xAAu
#define WDFEED_SEGUNDA      0x55u

void hal_wdt_iniciar(uint32_t timeout_ms){
	uint32_t ticks = (HAL_WDT_PCLK_HZ / 4u / 1000u) * timeout_ms;
	if(ticks < WDT_MINIMO_TICKS) ticks = WDT_MINIMO_TICKS;

	WDTC  = ticks;
	WDMOD = WDMOD_WDEN | WDMOD_WDRESET;
	hal_wdt_alimentar();                 /* la primera alimentacion arranca la cuenta */
}

void hal_wdt_alimentar(void){
	/* Las dos escrituras deben ser consecutivas: sin IRQ entre ellas (Paso 7).
	 * En armcc para ARM7, __disable_irq() devuelve el estado previo del bit I,
	 * asi se restaura el nivel que habia al entrar (no se habilita siempre).
	 * Si ya tienes el modulo de secciones criticas, sustituye estas lineas
	 * por sus funciones de entrar/salir. */
	int irq_estaban_deshabilitadas = hal_irq_estado();
	hal_irq_desactivar();
	WDFEED = WDFEED_PRIMERA;
	WDFEED = WDFEED_SEGUNDA;
	if(!irq_estaban_deshabilitadas) hal_irq_activar(irq_estaban_deshabilitadas);
}
