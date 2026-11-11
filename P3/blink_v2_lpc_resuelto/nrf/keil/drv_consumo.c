#include "drv_consumo.h"
#include "hal_consumo.h"
#include "drv_monitor.h"
#include <stdint.h>

static volatile uint32_t s_monitor;


void drv_consumo_iniciar(uint32_t id) {
	s_monitor = id;                 // Guarda el identificador asignado para el monitor
	hal_consumo_iniciar();          // Inicializa el hardware subyacente de gestión de consumo
	drv_monitor_marcar(s_monitor);  // Registra/marca en el monitor que el módulo ha iniciado su actividad
}


void drv_consumo_esperar(void) {
	drv_monitor_desmarcar(s_monitor); // Desmarca la monitorización (el módulo entra en pausa/espera)
	hal_consumo_esperar();            // Llama a la función del hardware para poner el sistema en espera
	drv_monitor_marcar(s_monitor);    // Vuelve a marcar la monitorización al salir del estado de espera
}

void drv_consumo_dormir(void) {
	hal_consumo_dormir(); // Pone el hardware en estado de reposo/sueño (sleep mode)
}
