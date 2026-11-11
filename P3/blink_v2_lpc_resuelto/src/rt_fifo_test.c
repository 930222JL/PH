#include "rt_fifo.h"
#include "rt_fifo_test.h"
#include "board.h"
#include <stdint.h>

// Prueba el comportamiento básico circular, el acceso FIFO, las estadísticas y la consistencia de tiempos
static uint32_t test_circular(void) {
	EVENTO_T id;
	uint32_t data;
	Tiempo_us_t TS;
	
	Tiempo_us_t t_anterior = 0; // Almacena el timestamp del evento anterior para verificar monotonicidad
	
	// Inicializa la cola FIFO usando el monitor asignado (MONITOR4)
	rt_FIFO_inicializar(MONITOR4);
	
	// PRUEBA 1: Encolar y extraer alternadamente múltiples veces (superando el tamaño máximo del buffer)
	for (uint16_t i = 0; i < 5 * RT_FIFO_TAM; i++) {
		rt_FIFO_encolar(ev_PULSAR_BOTON, i);
		
		// Debe devolver 1 elemento pendiente en la cola tras extraer
		if (rt_FIFO_extraer(&id, &data, &TS) != 1) return 0;
		
		// Comprueba que los datos extraídos coinciden con los que se acaban de encolar
		if (id != ev_PULSAR_BOTON || data != i) return 0;
		
		// El timestamp actual no puede ser inferior al del evento previo
		if (TS < t_anterior) return 0;
		t_anterior = TS;
	}
	
	// Comprueba que los contadores estadísticos globales y por tipo coincidan con el total encolado
	if (rt_FIFO_estadisticas(ev_VOID) != 5 * RT_FIFO_TAM) return 0;
	if (rt_FIFO_estadisticas(ev_PULSAR_BOTON) != 5 * RT_FIFO_TAM) return 0;
	
	// PRUEBA 2: Llenar la cola por completo
	for (uint16_t i = 0; i < RT_FIFO_TAM; i++) {
		rt_FIFO_encolar(ev_PULSAR_BOTON, i);
	}
	
	// Extraer todos los elementos uno a uno verificando orden FIFO (First-In, First-Out)
	for (uint16_t i = 0; i < RT_FIFO_TAM; i++) {
		// rt_FIFO_extraer debe retornar la cantidad de elementos pendientes ANTES de la extracción
		if (rt_FIFO_extraer(&id, &data, &TS) != (RT_FIFO_TAM - i)) return 0;
		if (data != i) return 0;
	}
	
	// Intenta extraer de una cola vacía; debe devolver 0 elementos disponibles
	if (rt_FIFO_extraer(&id, &data, &TS) != 0) return 0;
	
	return 1; // Test completado con éxito
}

// Prueba el comportamiento ante desbordamiento de la FIFO (overflow)
static uint32_t test_overflow(void) {
	rt_FIFO_inicializar(MONITOR4);
	
	// Intenta encolar RT_FIFO_TAM + 1 elementos para provocar el desbordamiento.
	// La función rt_FIFO_encolar debería entrar en bucle infinito/assert al intentar guardar el último elemento.
	for (uint16_t i = 0; i <= RT_FIFO_TAM; i++) {
		rt_FIFO_encolar(ev_PULSAR_BOTON, i);
	}
	
	// Si llega a esta línea significa que la prueba falló (no se bloqueó tras el desbordamiento)
	return 0;
}

// Función principal de ejecución del conjunto de pruebas
void test(void) {
	uint16_t ok = test_circular(); // Ejecuta la prueba de funcionamiento circular
	ok = test_overflow();          // Ejecuta la prueba de desbordamiento (bloqueará la ejecución)
}


	
	
