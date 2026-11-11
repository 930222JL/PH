#include "rt_fifo.h"
#include <stddef.h>
#include <nrf.h>
#include "drv_tiempo.h"
#include "drv_monitor.h"
#include "board.h"      

// Estructura que define la información que almacena cada evento en la cola
typedef struct {
	EVENTO_T ID_evento;   // Identificador o tipo de evento
	uint32_t auxData;     // Datos adicionales/payload asociados al evento
	Tiempo_us_t TS;       // Timestamp (marca de tiempo en microsegundos) en que se generó
} EVENTO;

// Variables de estado de la cola FIFO (marcadas como volatile al modificarse en interrupciones)
static volatile uint16_t siguiente_a_escribir, siguiente_a_tratar, pendientes, total_eventos;

// ID para el monitor en caso de que la cola sufra un desbordamiento (overflow)
static volatile uint32_t s_monitor;

// Buffer circular de eventos que representa la cola FIFO
EVENTO rt_FIFO[RT_FIFO_TAM];

// Array de contadores estadísticos para registrar la cantidad total de cada tipo de evento
uint32_t total_por_evento[EVENT_TYPES];

// Inicializa la estructura de la cola FIFO, sus punteros y estadísticas
void rt_FIFO_inicializar(uint32_t monitor_overflow) {
	s_monitor = monitor_overflow;
	drv_monitor_desmarcar(s_monitor); // Asegura que no haya marca de error de overflow al inicio

	// Reinicia todos los índices y contadores
	siguiente_a_escribir = siguiente_a_tratar = pendientes = total_eventos = 0;

	// Limpia la tabla de estadísticas por evento
	for (uint16_t i = 0; i < EVENT_TYPES; i++) {
		total_por_evento[i] = 0;
	}

	// Vacía todas las posiciones del buffer de la FIFO colocándolas en estado nulo (ev_VOID)
	for (uint16_t i = 0; i < RT_FIFO_TAM; i++) {
		rt_FIFO[i].ID_evento = ev_VOID;
		rt_FIFO[i].auxData = 0;
		rt_FIFO[i].TS = 0;
	}
}

// Inserta un nuevo evento en la cola FIFO
void rt_FIFO_encolar(EVENTO_T ID_evento, uint32_t auxData) {
	// Guarda el estado actual del registro PRIMASK y deshabilita interrupciones
	// (Sección crítica para evitar carreras ya que se llama desde ISR y main)
	uint32_t primask = __get_PRIMASK();
	__disable_irq();

	// Manejo de desbordamiento: si la cola está llena, marca el error en el monitor y bloquea el sistema
	if (pendientes >= RT_FIFO_TAM) {
		drv_monitor_marcar(s_monitor);
		while (1); // Bucle infinito tras un desbordamiento crítico de la FIFO
	}

	// Guarda la información del nuevo evento en la posición actual de escritura
	rt_FIFO[siguiente_a_escribir].ID_evento = ID_evento;
	rt_FIFO[siguiente_a_escribir].auxData = auxData;
	rt_FIFO[siguiente_a_escribir].TS = drv_tiempo_actual_us();

	// Avanza el índice de escritura de forma circular
	siguiente_a_escribir = (siguiente_a_escribir + 1) % RT_FIFO_TAM;

	// Actualiza contadores globales
	pendientes++;
	total_eventos++;
	if (ID_evento < EVENT_TYPES) {
		total_por_evento[ID_evento]++;
	}

	// Restaura el estado previo de las interrupciones
	__set_PRIMASK(primask);
}

// Extrae el evento más antiguo de la cola FIFO (devuelve >0 si había elementos)
uint8_t rt_FIFO_extraer(EVENTO_T *ID_evento, uint32_t *auxData, Tiempo_us_t *TS) {
	// Guarda el estado previo de las interrupciones y entra en sección crítica
	uint32_t primask = __get_PRIMASK();
	uint8_t hay;
	__disable_irq();

	hay = (uint8_t)pendientes; // Guarda la cantidad de elementos pendientes antes de extraer

	// Si hay elementos disponibles en la cola, copia los datos al exterior
	if (pendientes > 0) {
		*ID_evento = rt_FIFO[siguiente_a_tratar].ID_evento;
		*auxData   = rt_FIFO[siguiente_a_tratar].auxData;
		*TS        = rt_FIFO[siguiente_a_tratar].TS;

		// Avanza el índice de lectura de forma circular
		siguiente_a_tratar = (siguiente_a_tratar + 1) % RT_FIFO_TAM;
		pendientes--;
	}

	// Restaura el estado de las interrupciones
	__set_PRIMASK(primask);
	return hay;
}

// Consulta las estadísticas de eventos registrados según el ID proporcionado
uint32_t rt_FIFO_estadisticas(EVENTO_T ID_evento) {
	if (ID_evento == ev_VOID) return total_eventos;            // Devuelve el global acumulado
	if (ID_evento < EVENT_TYPES) return total_por_evento[ID_evento]; // Devuelve el acumulado de ese evento
	return 0;                                                   // ID fuera de rango
}

uint32_t rt_FIFO_estadisticas(EVENTO_T ID_evento){
	if(ID_evento == ev_VOID) return total_eventos;
	if(ID_evento < EVENT_TYPES) return total_por_evento[ID_evento];
	return 0;
}
