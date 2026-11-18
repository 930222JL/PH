/* *****************************************************************************
 * hal_wdt.h: interfaz HAL del watchdog (independiente de la plataforma).
 * Implementaciones: hal_wdt_nrf.c (nRF52840) y hal_wdt_lpc.c (LPC2105).
 * ****************************************************************************/
#ifndef HAL_WDT_H
#define HAL_WDT_H

#include <stdint.h>

/* Configura y arranca el watchdog. Si en timeout_ms no se alimenta, resetea.
 * Una vez arrancado NO se puede parar (solo un reset lo detiene). */
void hal_wdt_iniciar(uint32_t timeout_ms);

/* Alimenta al watchdog (reinicia su cuenta). */
void hal_wdt_alimentar(void);

#endif
