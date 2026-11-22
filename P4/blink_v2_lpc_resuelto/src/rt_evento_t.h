#ifndef RT_EVENTO_T_H
#define RT_EVENTO_T_H

typedef enum{
	ev_VOID =0,
	ev_T_PERIODICO = 1,
	ev_PULSAR_BOTON = 2,
	ev_A_PERIODICA=3,
	ev_BOTON_RETARDO=4,
	ev_BOTON_MUESTREO=5,
	ev_BOTON_REBOTES=6,
	ev_INACTIVIDAD=7,
} EVENTO_T;

#define EVENT_TYPES 8

#endif

