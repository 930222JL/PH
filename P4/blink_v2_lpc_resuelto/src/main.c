/* *****************************************************************************
 * P.H.2025: main.c
 *
 * Programa principal de la práctica 2 de Proyecto Hardware.
 *
 * - Inicializa la HAL de GPIO y el driver de LEDs.
 * - Invoca a las funciones de parpadeo (blink_v1 o blink_v2) según la sesión.
 *
 * Versión de prácticas:
 *   - Sesión 1: blink_v1 -> parpadeo con retardo por bucle de instrucciones.
 *   - Sesión 2: blink_v2 -> parpadeo controlado por temporizador.
 *
 * Notas:
 *   - El identificador de LED comienza en 1 (primer LED definido en board.h).
 *   - El bucle principal no retorna (sistema embebido).
 *
 * Autores: Enrique Torrs
 * Universidad de Zaragoza
 * ****************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <stdbool.h>
#include "board.h"

#include "hal_gpio.h"
#include "drv_leds.h"
#include "drv_tiempo.h"
#include "drv_consumo.h"
#include "drv_monitor.h"
#include "svc_GE.h"
#include "rt_GE.h"
#include "drv_botones.h"
#include "rt_fifo.h"
#include "rt_fifo_test.h"

/* Constante de retardo en milisegundos */
#define RETARDO_MS 500u 		 /* ejemplo: medio segundo */
#define USAR_BLINK_V2 1
/* Prototipos */
static void blink_v1(LED_id_t id);
static void blink_v2(LED_id_t id);

static volatile bool pulsado=false;
static volatile uint16_t counter_sequence[4]={1,3,2,4};

/* *****************************************************************************
 * BLINK v1: parpadeo de un LED conmutando on/off.
 * Retardo por bucle de instrucciones (busy-wait). Solo usa el driver de LEDs.
 * para realizar la primera sesión de la practica
 */
static void blink_v1(LED_id_t id) {

	drv_led_establecer(id, LED_ON);
    while (1) {
        volatile uint32_t tmo = 20000000u;  /* ajustad para vuestro reloj */
        while (tmo--) { /* nop */ }
        (void)drv_led_conmutar(id);
    }
}

/* *****************************************************************************
 * BLINK v2, parpadeo de un led conmutando on/off 
 * activacion por tiempo, usa tanto manejador del led como el del tiempo
 * para realizar en la segunda sesion de la practica, version a entregar
 */
static void blink_v2(LED_id_t id) {
    Tiempo_ms_t siguiente_activacion;	

    /* Encender LED inicial */
    drv_led_establecer(id, LED_ON);

    /* Tomar tiempo de referencia */
    siguiente_activacion = drv_tiempo_actual_ms();
	
    /* Bucle principal: conmutar con temporizador */
    while (true) {
        siguiente_activacion += RETARDO_MS;     /* calcular la siguiente activación */
        drv_tiempo_esperar_hasta_ms(siguiente_activacion);
        (void)drv_led_conmutar(id);

        /* Aquí podrían añadirse otras tareas periódicas */
    }
}

void leds_c(uint32_t id){
	drv_led_conmutar(id);
}

static void blink_v3(LED_id_t id) {
    /* Encender LED inicial */
    drv_led_establecer(id, LED_ON);

    /* Tomar tiempo de referencia */
    drv_tiempo_periodico_ms(500,leds_c,id);
	
    /* Bucle principal: conmutar con temporizador */
    while (true) {
        drv_consumo_esperar();

        /* Aquí podrían añadirse otras tareas periódicas */
    }
}

static void blink_v3_bis(LED_id_t id) {
    /* Encender LED inicial */
    drv_led_establecer(id, LED_ON);

    /* Tomar tiempo de referencia */
    drv_tiempo_periodico_ms(500,leds_c,id);
	
    /* Bucle principal: conmutar con temporizador */
		uint16_t n=0;
    while (n<20) {
				n++;
        drv_consumo_esperar();

        /* Aquí podrían añadirse otras tareas periódicas */
    }
		drv_consumo_dormir();
}

static void blink_v4(LED_id_t id) {
		EVENTO_T EV_ID_evento;
		uint32_t EV_auxData;
		Tiempo_us_t EV_TS;
		
    rt_FIFO_inicializar(MONITOR4);
	
		/* Encender LED inicial */
    drv_led_establecer(id, LED_ON);

    /* Tomar tiempo de referencia */
    drv_tiempo_periodico_ms(500,rt_FIFO_encolar,ev_T_PERIODICO);
	
    /* Bucle principal: conmutar con temporizador */
    while (true) {
				if(rt_FIFO_extraer(&EV_ID_evento,&EV_auxData,&EV_TS)){
					if(ev_T_PERIODICO)drv_led_conmutar(id);
				}else{
					drv_consumo_esperar();
				}

        /* Aquí podrían añadirse otras tareas periódicas */
    }
}

static void pulsar(){
	pulsado=true;
}

static void bit_counter_strike(){
	svc_GE_suscribir(ev_PULSAR_BOTON,0,pulsar);
	uint16_t n=0;
	while(1){
		if(n>=4)n=0;
		drv_led_establecer(counter_sequence[n],1);
		while(!pulsado);
		pulsado=false;
		drv_led_establecer(counter_sequence[n],0);
		n++;
	}
}

/* *****************************************************************************
 * MAIN, Programa principal.
 * para la primera sesion se debe usar la funcion de blink_v1 sin temporizadores
 * para la entrega final se debe incocar blink_v2
 */
int main(void){
	uint32_t Num_Leds;

  /* Init tiempo, es un reloj que indica el tiempo desde que comenzo la ejecución */
	bool error = drv_tiempo_iniciar(); // para la sesion 2 de practica 2
	
	hal_gpio_iniciar();	// llamamos a iniciar gpio antesde que lo hagan los drivers
	
	/* Configurar LEDs (IDs válidos: 1..num_leds) */
	Num_Leds = drv_leds_iniciar();
	drv_monitor_iniciar();
	drv_consumo_iniciar(MONITOR3);
	
	
	if (Num_Leds > 0){
			/* Sesión 1: parpadeo por lazo ocupado */
			//blink_v1((LED_id_t)1);

			/* Sesión 2: temporizador (cuando esté disponible)
			 */
			//blink_v2((LED_id_t)2);
			//blink_v3((LED_id_t)3);
			blink_v4((LED_id_t)4);
			//test();
    }

    /* En sistemas empotrados normalmente no se retorna */
    while (1) { /* idle */ }
    /* return 0; */
}
