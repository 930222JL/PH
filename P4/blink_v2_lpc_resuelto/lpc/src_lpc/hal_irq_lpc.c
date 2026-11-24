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
		uint32_t estado,aux;
	__asm{
		MRS estado, CPSR
		ORR aux,estado,#0x80
		MSR CPSR_cxsf, aux
	}
}

void hal_irq_activar(uint32_t estado){
	__asm{
		MSR CPSR_cxsf, estado
	}
}