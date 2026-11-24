#include "hal_ext_int.h"
#include "rt_evento_t.h"
#include <LPC210x.H>
#include <stdbool.h>

#define NUM_EINT 3

static volatile uint32_t pin_vic[NUM_EINT]={14,15,16};
static volatile uint32_t pin_eint[NUM_EINT]={16,14,15};
static volatile bool configurado[NUM_EINT];

static void (*f_cb)(EVENTO_T ID_evento,uint32_t pin);

void eint0_isr(void) __irq{
	VICIntEnClr=(1 << pin_vic[0]);
	EXTINT=(1U<<0);
	if(f_cb)f_cb(ev_PULSAR_BOTON,pin_eint[0]);
	VICVectAddr=0;
}

void eint1_isr(void) __irq{
	VICIntEnClr=(1 << pin_vic[1]);
	EXTINT=(1U<<1);
	if(f_cb)f_cb(ev_PULSAR_BOTON,pin_eint[1]);
	VICVectAddr=0;
}

void eint2_isr(void) __irq{
	VICIntEnClr=(1 << pin_vic[2]);
	EXTINT=(1U<<2);
	if(f_cb)f_cb(ev_PULSAR_BOTON,pin_eint[2]);
	VICVectAddr=0;
}

void hal_ext_int_iniciar(void (*f_callback_isr)(EVENTO_T ID_evento,uint32_t pin)){
	f_cb=f_callback_isr;
	for(uint16_t i=0;i<NUM_EINT;i++){
		configurado[i]=false;
		hal_ext_int_config_pin(i);
	}
}

void hal_ext_int_config_pin(uint32_t pin){
	switch(pin){
		case 0:
			PINSEL1= (PINSEL1 | (1U << 0));
			VICVectAddr2=(unsigned long)eint0_isr;
			VICVectCntl2=(1<<5) | pin_vic[0];
			configurado[0]=true;
			break;
		
		case 1:
			PINSEL0= (PINSEL0 | (2U << 28));
			VICVectAddr3=(unsigned long)eint1_isr;
			VICVectCntl3=(1<<5) | pin_vic[1];
			configurado[1]=true;
			break;
		
		case 2:
			PINSEL0= (PINSEL0 | (2U << 30));
			VICVectAddr4=(unsigned long)eint2_isr;
			VICVectCntl4=(1<<5) | pin_vic[2];
			configurado[2]=true;
			break;
		
		default: 
			break;
	}
	EXTINT=(1U<<pin);
	EXTWAKE=(1U<<pin);
	VICIntEnable=(1U<<pin_vic[pin]);
}
void hal_ext_int_deshabilitar(uint32_t pin){
	VICIntEnClr=(1U<<pin);
}
void hal_ext_int_habilitar(uint32_t pin){
	EXTINT=(1U<<pin_vic[pin]);
	VICIntEnable=(1U<<pin);
}
