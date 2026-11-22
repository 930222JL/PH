#include "hal_irq.h"
#include "board.h"

uint32_t hal_irq_estado(void){
	uint32_t estado;
	__asm{
		MRS estado, CPSR
	}
	return estado;
}

void hal_irq_desactivar(void){
	__asm{
		MRS r0, CPSR
		ORR r0,r0,#0x80
		MSR CPSR, r0
	}
}

void hal_irq_activar(uint32_t estado){
	__asm{
		MSR CPSR, estado
	}
}