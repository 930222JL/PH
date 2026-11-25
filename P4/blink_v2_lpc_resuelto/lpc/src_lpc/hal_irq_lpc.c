#include "hal_irq.h"
#include "board.h"

/* Lee y devuelve el valor actual del registro CPSR. */
uint32_t hal_irq_estado(void){
	uint32_t estado;
	__asm{
		MRS estado, CPSR
	}
	return estado;
}

/* Pone a 1 el bit I (0x80) del CPSR para enmascarar (desactivar) las IRQ. */
void hal_irq_desactivar(void){
		uint32_t estado,aux;
	__asm{
		MRS estado, CPSR
		ORR aux,estado,#0x80
		MSR CPSR_cxsf, aux
	}
}

/* Restaura el registro CPSR completo al estado guardado previamente. */
void hal_irq_activar(uint32_t estado){
	__asm{
		MSR CPSR_cxsf, estado
	}
}
