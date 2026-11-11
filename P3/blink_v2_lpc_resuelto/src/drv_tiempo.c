/* *****************************************************************************
 * P.H.2025: Driver/Manejador de los temporizadores
 * suministra los servicios independientemente del hardware
 *
 * usa los servicos de hal_tiempo.h: 
 */
 
#include "drv_tiempo.h"
#include "hal_tiempo.h"

static volatile uint32_t ticks_us=0;  // Frecuencia del reloj: numero de ticks del hardware por microsegundo
static volatile uint32_t id_evento;   // Identificador del evento asignado al temporizador periodico

//inicializa el reloj y empieza a contar
bool drv_tiempo_iniciar(void){
	hal_tiempo_info_t info;
	hal_tiempo_iniciar_tick(&info);
	ticks_us=info.ticks_per_us;		//guardamos cuanto mide un tick para contar el tiempo
	return (info.ticks_per_us!=0);		//si un tick no vale nada no ha arrancado el timer
}

//tiempo desde que se inicio el temporizador en microsegundos
Tiempo_us_t drv_tiempo_actual_us(void){
	return (Tiempo_us_t)(hal_tiempo_actual_tick64()/ticks_us);		//sacamos el tiempo total en ticks y lo dividimos entre el tiempo que le cuesta un tick
}

//tiempo desde que se inicio el temporizador en milisegundos
Tiempo_ms_t drv_tiempo_actual_ms(void){
	return (Tiempo_ms_t)(hal_tiempo_actual_tick64() /((uint64_t)ticks_us *1000u));  // Convierte los ticks a ms dividiendo entre (ticks por ms)
}

//retardo: esperar un cierto tiempo en milisegundos
void drv_tiempo_esperar_ms(Tiempo_ms_t ms){
	uint64_t fin=hal_tiempo_actual_tick64()+(ms*ticks_us*1000u);		//sumamos a los ticks actuales el tiempo en ticks al que queremos llegar
	while(hal_tiempo_actual_tick64()<fin);													//esperamos a que los ticks actuales sean igual al tick al que queriamos esperar
}

//esperar hasta un determinado tiempo (en ms), devuelve el tiempo actual
Tiempo_ms_t drv_tiempo_esperar_hasta_ms(Tiempo_ms_t ms){
	Tiempo_ms_t ahora=drv_tiempo_actual_ms();
	while((int32_t)((uint32_t)ms-(uint32_t)ahora)>0){  // Bucle de espera activa usando resta con signo para evitar fallos por desbordamiento
		ahora=drv_tiempo_actual_ms();
	}
	return ahora;
}


static volatile uint32_t id_evento;                       // Redeclaracion de la variable global de evento para el callback
static void (*s_callback_app)(uint32_t) = 0;              // Puntero a la funcion callback recibida desde la aplicacion

/* Funcion que ve el HAL: sin parametros. Se ejecuta en la ISR de TIMER0 */
static void callback_intermedio(void){
	if (s_callback_app) s_callback_app(id_evento);        // Ejecuta la funcion de la aplicacion pasandole el ID del evento activo
}

void drv_tiempo_periodico_ms(Tiempo_ms_t ms, void(*funcion_callback_app)(uint32_t), uint32_t ID_evento){
	uint32_t periodo = (uint32_t)ms * ticks_us * 1000u;    // Convierte el tiempo recibido en ms a ticks de hardware

	id_evento = ID_evento;                                 // Guarda el ID del evento que se le pasara a la funcion callback
	s_callback_app = funcion_callback_app;                 // Almacena el puntero a la funcion de la aplicacion

	if (ms == 0 || funcion_callback_app == 0){
		hal_tiempo_reloj_periodico_tick(0, 0);             /* deshabilitar */
	} else {
		hal_tiempo_reloj_periodico_tick(periodo, callback_intermedio);  // Configura la alarma periodica del HAL con el callback intermedio
	}
}
