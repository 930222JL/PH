/* *****************************************************************************
 * P.H.2025: implementacion para cumplir el hal_tiempo.h (nRF52840)
 *
 * Tick libre con TIMER1: contador de 32 bits a 1 MHz (1 tick = 1 us).
 * Misma estructura que la version LPC: COMPARE0 = valor maximo (2^32-1),
 * el contador se reinicia solo al llegar (SHORTS) y la ISR cuenta las vueltas.
 * Tiempo en ticks = vueltas * 2^32 + contador.
 */
#include <nrf.h>
#include "hal_tiempo.h"

#define TICKS_PER_US      1u                 /* 16 MHz / 2^4 = 1 MHz */
#define COUNTER_BITS      32u
#define COUNTER_MAX       (0xFFFFFFFFu)

static volatile uint32_t s_overflows_t1 = 0;  /* cuenta desbordes de T1 */

/* RSI del TIMER1 */
void TIMER1_IRQHandler(void) {
	volatile uint32_t lectura;

	if (NRF_TIMER1->EVENTS_COMPARE[0] != 0) {
		NRF_TIMER1->EVENTS_COMPARE[0] = 0;
		lectura = NRF_TIMER1->EVENTS_COMPARE[0];   // relectura para asegurar el borrado 
		(void)lectura;
		s_overflows_t1++;                        /* una vuelta completa de 2^32 ticks */
	}
}

/* Inicializa el contador libre y devuelve los parametros de conversion */
void hal_tiempo_iniciar_tick(hal_tiempo_info_t *out_info){
	NRF_TIMER1->TASKS_STOP  = 1;																	//parar timer
	NRF_TIMER1->TASKS_CLEAR = 1;																	//poner timer a 0
	s_overflows_t1 = 0;

	NRF_TIMER1->MODE      = TIMER_MODE_MODE_Timer << TIMER_MODE_MODE_Pos;	//para contar pulsos de reloj
	NRF_TIMER1->BITMODE   = TIMER_BITMODE_BITMODE_32Bit << TIMER_BITMODE_BITMODE_Pos;	//contador a 32 bits
	NRF_TIMER1->PRESCALER = 4 << TIMER_PRESCALER_PRESCALER_Pos;      /* 16 MHz / 2^4 = 1 MHz */

	NRF_TIMER1->CC[0] = COUNTER_MAX;		//indicamos cada cuanto es una vuelta
	NRF_TIMER1->EVENTS_COMPARE[0] = 0;	//borra cualquier aviso de ese evento
	NRF_TIMER1->SHORTS   = TIMER_SHORTS_COMPARE0_CLEAR_Enabled << TIMER_SHORTS_COMPARE0_CLEAR_Pos;	//si salta compare0 se reinicia el contador automaticamente
	NRF_TIMER1->INTENCLR = 0xFFFFFFFFu;		//desactiva todas las interrupciones del timer
	NRF_TIMER1->INTENSET = TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos;		//activa las interrupciones del timer que nos interesan

	NVIC_ClearPendingIRQ(TIMER1_IRQn);		//borra si queda una interrupcion pendiente antes de arrancar
	NVIC_EnableIRQ(TIMER1_IRQn);		//le indicamos al procesador que no ignore las interrupciones
	NRF_TIMER1->TASKS_START = 1;		//arranca el timer

	out_info->ticks_per_us = TICKS_PER_US;
	out_info->counter_bits = COUNTER_BITS;
	out_info->counter_max  = COUNTER_MAX;
}

static uint32_t leer_contador(void){
	NRF_TIMER1->TASKS_CAPTURE[1] = 1;
	return NRF_TIMER1->CC[1];
}

/* lectura que para interrupciones, lee el contador y comprueba si salto interrupcion y no dio tiempo a servirla para contar la vuelta y servir la 
interrupcion en la funcion, reactiva interrupciones y devuelve tiempo*/
uint64_t hal_tiempo_actual_tick64(void) {
	uint32_t primask = __get_PRIMASK();		//guardamos estado de las interrupciones
	uint32_t lo, hi;

	__disable_irq();											// desactivamos interrupciones para no dejar la operacion a mitad
	lo = leer_contador();
	hi = s_overflows_t1;
	if (NRF_TIMER1->EVENTS_COMPARE[0] != 0 && lo < 0x80000000u) {	/*si ha saltado interrupcion y el valor de tick es menor que la mitad no se ha servido 
																																*/la interrupcion
		hi++;
	}
	__set_PRIMASK(primask);							// devolvemos las interrupciones al estado en el que estaban antes de desactivarlas

	return (((uint64_t)hi) << 32) | lo;
}


static void (*s_cb)() = 0;

void TIMER0_IRQHandler(void) {
	if(s_cb) s_cb();
	volatile uint32_t dummy;
	NRF_TIMER1->EVENTS_COMPARE[0] = 0;
	dummy = NRF_TIMER1->EVENTS_COMPARE[0];   /* relectura para asegurar el borrado */
	(void)dummy;
	                        /* una vuelta completa de 2^32 ticks */
}

static void hal_tiempo_periodico_set_callback(void (*cb)()){
	s_cb=ch;
}

static void hal_tiempo_periodico_config_tick(uint32_t periodo_en_tick){
	NRF_TIMER0->TASKS_STOP  = 1;
	if(periodo_en_tick==0)return;
	NRF_TIMER0->MODE      = TIMER_MODE_MODE_Timer << TIMER_MODE_MODE_Pos;	//para contar pulsos de reloj
	NRF_TIMER0->BITMODE   = TIMER_BITMODE_BITMODE_32Bit << TIMER_BITMODE_BITMODE_Pos;	//contador a 32 bits
	NRF_TIMER0->PRESCALER = 4 << TIMER_PRESCALER_PRESCALER_Pos;      /* 16 MHz / 2^4 = 1 MHz */
	NRF_TIMER0->CC[0] = periodo_en_tick-1;
	NRF_TIMER0->SHORTS   = TIMER_SHORTS_COMPARE0_CLEAR_Enabled << TIMER_SHORTS_COMPARE0_CLEAR_Pos;
	NRF_TIMER0->INTENSET = TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos;
}
	

static void hal_tiempo_periodico_enable(bool enable){
	if(enable){
		NRF_TIMER0->TASKS_CLEAR = 1;
		NRF_TIMER0->INTENSET = TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos;
		NVIC_EnableIRQ(TIMER0_IRQn);
		NRF_TIMER0->TASKS_START = 1;
	}else{
		NRF_TIMER0->TASKS_STOP  = 1;
		NRF_TIMER0->INTENCLR = 0xFFFFFFFFu;
		NVIC_ClearPendingIRQ(TIMER0_IRQn);
	}
}

void hal_tiempo_reloj_periodico_tick(uint32_t periodo_en_tick, void (*funcion_callback_drv)(void)){
    /* 1) Deshabilitar antes de tocar config/callback (evita ISR en mitad de cambios) */
    hal_tiempo_periodico_enable(false);

    /* 2) Registrar callback (puede ser NULL si vamos a deshabilitar) */
    hal_tiempo_periodico_set_callback(funcion_callback_drv);

    /* 3) Configurar periodo (no arranca el temporizador) */
    hal_tiempo_periodico_config_tick(periodo_en_tick);

    /* 4) Habilitar si hay periodo válido y callback no nulo; deshabilitar en caso contrario */
    if (periodo_en_tick != 0u && funcion_callback_drv != NULL) {
        hal_tiempo_periodico_enable(true);
    } else {
        hal_tiempo_periodico_enable(false);
    }
}